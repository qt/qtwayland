/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandeglclientbufferintegration.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <qpa/qplatformscreen.h>
#include <QtGui/QWindow>
#include <QtCore/QPointer>
#include <QDebug>

#include <QMutex>
#include <QMutexLocker>
#include <QtCore/private/qcore_unix_p.h>
#include <QtPlatformSupport/private/qeglstreamconvenience_p.h>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES     0x8D65
#endif

#ifndef EGL_WAYLAND_BUFFER_WL
#define EGL_WAYLAND_BUFFER_WL       0x31D5
#endif

#ifndef EGL_WAYLAND_PLANE_WL
#define EGL_WAYLAND_PLANE_WL        0x31D6
#endif

#ifndef EGL_WAYLAND_Y_INVERTED_WL
#define EGL_WAYLAND_Y_INVERTED_WL   0x31DB
#endif

#ifndef EGL_TEXTURE_RGB
#define EGL_TEXTURE_RGB             0x305D
#endif

#ifndef EGL_TEXTURE_RGBA
#define EGL_TEXTURE_RGBA            0x305E
#endif

#ifndef EGL_TEXTURE_EXTERNAL_WL
#define EGL_TEXTURE_EXTERNAL_WL     0x31DA
#endif

#ifndef EGL_TEXTURE_Y_U_V_WL
#define EGL_TEXTURE_Y_U_V_WL        0x31D7
#endif

#ifndef EGL_TEXTURE_Y_UV_WL
#define EGL_TEXTURE_Y_UV_WL         0x31D8
#endif

#ifndef EGL_TEXTURE_Y_XUXV_WL
#define EGL_TEXTURE_Y_XUXV_WL       0x31D9
#endif

/* Needed for compatibility with Mesa older than 10.0. */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL_compat) (EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);

#ifndef EGL_WL_bind_wayland_display
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
#endif

#ifndef EGL_KHR_image
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);
#endif

QT_BEGIN_NAMESPACE

struct BufferState
{
    BufferState();

    EGLint egl_format;
    QVarLengthArray<EGLImageKHR, 3> egl_images;
    EGLStreamKHR egl_stream;
    GLuint eglstream_texture;

    bool isYInverted;
    QSize size;
};

class WaylandEglClientBufferIntegrationPrivate
{
public:
    WaylandEglClientBufferIntegrationPrivate();

    void attach(struct ::wl_resource *buffer);
    void attach_egl_texture(struct ::wl_resource *buffer, EGLint format);
    void attach_egl_fd_texture(struct ::wl_resource *buffer, EGLNativeFileDescriptorKHR streamFd);
    void register_buffer(struct ::wl_resource *buffer, BufferState state);
    void bindBuffer(struct ::wl_resource *buffer);

    static void handle_buffer_destroy(wl_listener *listener, void *data);

    EGLDisplay egl_display;
    bool valid;
    bool display_bound;
    QHash<struct ::wl_resource *, BufferState> buffers;

    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display;
    PFNEGLQUERYWAYLANDBUFFERWL_compat egl_query_wayland_buffer;

    PFNEGLCREATEIMAGEKHRPROC egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d;

    QEGLStreamConvenience *funcs;
};

struct buffer_destroy_listener : wl_listener
{
    buffer_destroy_listener()
        : d(0)
    {
        notify = WaylandEglClientBufferIntegrationPrivate::handle_buffer_destroy;
        wl_list_init(&this->link);
    }

    WaylandEglClientBufferIntegrationPrivate *d;
};

BufferState::BufferState()
    : egl_format(EGL_TEXTURE_RGBA)
    , egl_stream(EGL_NO_STREAM_KHR)
    , eglstream_texture(0)
    , isYInverted(true)
{}

WaylandEglClientBufferIntegrationPrivate::WaylandEglClientBufferIntegrationPrivate()
    : egl_display(EGL_NO_DISPLAY)
    , valid(false)
    , display_bound(false)
    , egl_bind_wayland_display(0)
    , egl_unbind_wayland_display(0)
    , egl_query_wayland_buffer(0)
    , egl_create_image(0)
    , egl_destroy_image(0)
    , gl_egl_image_target_texture_2d(0)
    , funcs(Q_NULLPTR)
{}

void WaylandEglClientBufferIntegrationPrivate::attach(struct ::wl_resource *buffer)
{
    EGLint format;
    EGLNativeFileDescriptorKHR streamFd = EGL_NO_FILE_DESCRIPTOR_KHR;

    if (egl_query_wayland_buffer(egl_display, buffer, EGL_TEXTURE_FORMAT, &format))
        attach_egl_texture(buffer, format);
    else if (egl_query_wayland_buffer(egl_display, buffer, EGL_WAYLAND_BUFFER_WL, &streamFd))
        attach_egl_fd_texture(buffer, streamFd);
}

void WaylandEglClientBufferIntegrationPrivate::attach_egl_texture(struct ::wl_resource *buffer, EGLint format)
{
    // Resolving GL functions may need a context current, so do it only here.
    if (!gl_egl_image_target_texture_2d)
        gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

    if (!gl_egl_image_target_texture_2d) {
        qWarning("QtCompositor: bindTextureToBuffer() failed. Could not find glEGLImageTargetTexture2DOES.");
        return;
    }

    BufferState state;
    state.egl_format = format;

#if defined(EGL_WAYLAND_Y_INVERTED_WL)
    EGLint isYInverted;
    EGLBoolean ret = egl_query_wayland_buffer(egl_display, buffer, EGL_WAYLAND_Y_INVERTED_WL, &isYInverted);
    // Yes, this looks strange, but the specification says that EGL_FALSE return
    // value (not supported) should be treated the same as EGL_TRUE return value
    // and EGL_TRUE in value.
    state.isYInverted = (ret == EGL_FALSE || isYInverted == EGL_TRUE);
#endif

    int planes = 1;

    switch (format) {
    default:
    case EGL_TEXTURE_RGB:
    case EGL_TEXTURE_RGBA:
    case EGL_TEXTURE_EXTERNAL_WL:
        planes = 1;
        break;
    case EGL_TEXTURE_Y_UV_WL:
        planes = 2;
        break;
    case EGL_TEXTURE_Y_U_V_WL:
        planes = 3;
        break;
    case EGL_TEXTURE_Y_XUXV_WL:
        planes = 2;
        break;
    }

    for (int i = 0; i < planes; i++) {
        const EGLint attribs[] = { EGL_WAYLAND_PLANE_WL, i, EGL_NONE };
        EGLImageKHR image = egl_create_image(egl_display,
                                             EGL_NO_CONTEXT,
                                             EGL_WAYLAND_BUFFER_WL,
                                             buffer,
                                             attribs);

        if (image == EGL_NO_IMAGE_KHR)
            qWarning("failed to create EGL image for plane %d", i);

        state.egl_images << image;
    }

    register_buffer(buffer, state);
}

void WaylandEglClientBufferIntegrationPrivate::attach_egl_fd_texture(struct ::wl_resource *buffer, EGLNativeFileDescriptorKHR streamFd)
{
    BufferState state;

    state.egl_format = EGL_TEXTURE_EXTERNAL_WL;
    state.isYInverted = false;
    state.egl_stream = funcs->create_stream_from_file_descriptor(egl_display, streamFd);

    close(streamFd);

    if (state.egl_stream == EGL_NO_STREAM_KHR) {
        qWarning("%s:%d: eglCreateStreamFromFileDescriptorKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
        return;
    }

    if (!QOpenGLContext::currentContext())
        qWarning("EglClientBufferIntegration: creating texture with no current context");

    //TODO This texture might end up in a different context than the quick item which wants to use it, this needs to be fixed somehow.
    glGenTextures(1, &state.eglstream_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, state.eglstream_texture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    register_buffer(buffer, state);

    bindBuffer(buffer);
}

void WaylandEglClientBufferIntegrationPrivate::register_buffer(struct ::wl_resource *buffer, BufferState state)
{
    Q_ASSERT(!buffers.contains(buffer));

    EGLint width, height;
    egl_query_wayland_buffer(egl_display, buffer, EGL_WIDTH, &width);
    egl_query_wayland_buffer(egl_display, buffer, EGL_HEIGHT, &height);
    state.size = QSize(width, height);

    buffers[buffer] = state;

    buffer_destroy_listener *destroy_listener = new buffer_destroy_listener;
    destroy_listener->d = this;
    wl_signal_add(&buffer->destroy_signal, destroy_listener);
}

void WaylandEglClientBufferIntegrationPrivate::bindBuffer(struct ::wl_resource *buffer)
{
    if (!valid) {
        qWarning("QtCompositor: bindTextureToBuffer() failed");
        return;
    }

    if (!buffer || !buffers.contains(buffer))
        return;

    const BufferState state = buffers.value(buffer);

    if (state.egl_stream != EGL_NO_STREAM_KHR) {
        EGLint stream_state;
        funcs->query_stream(egl_display, state.egl_stream, EGL_STREAM_STATE_KHR, &stream_state);

        if (stream_state == EGL_STREAM_STATE_CREATED_KHR)
            if (funcs->stream_consumer_gltexture(egl_display, state.egl_stream) != EGL_TRUE)
                qWarning("%s:%d: eglStreamConsumerGLTextureExternalKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
    } else {
        GLint previousTexture = GL_TEXTURE0;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &previousTexture);

        const GLenum target = (state.egl_format == EGL_TEXTURE_EXTERNAL_WL) ? GL_TEXTURE_EXTERNAL_OES
                                                                            : GL_TEXTURE_2D;

        for (int i = 0; i < state.egl_images.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            gl_egl_image_target_texture_2d(target, state.egl_images[i]);
        }

        glActiveTexture(previousTexture);
    }
}

void WaylandEglClientBufferIntegrationPrivate::handle_buffer_destroy(wl_listener *listener, void *data)
{
    buffer_destroy_listener *destroy_listener = static_cast<buffer_destroy_listener *>(listener);
    WaylandEglClientBufferIntegrationPrivate *self = destroy_listener->d;
    struct ::wl_resource *buffer = static_cast<struct ::wl_resource *>(data);

    wl_list_remove(&destroy_listener->link);
    delete destroy_listener;

    if (!self->buffers.contains(buffer))
        return;

    Q_ASSERT(self);
    Q_ASSERT(buffer);

    BufferState state = self->buffers.take(buffer);

    // We would need to delete the texture of the egl_stream here, but we can't as this breaks the
    // texture of the new stream when the window is resized. It seems wayland takes care to delete the texture for us.

    for (int i = 0; i < state.egl_images.size(); i++)
        self->egl_destroy_image(self->egl_display, state.egl_images[i]);

    if (state.egl_stream != EGL_NO_STREAM_KHR)
        self->funcs->destroy_stream(self->egl_display, state.egl_stream);
}

WaylandEglClientBufferIntegration::WaylandEglClientBufferIntegration()
    : QtWayland::ClientBufferIntegration()
    , d_ptr(new WaylandEglClientBufferIntegrationPrivate)
{
}

void WaylandEglClientBufferIntegration::initializeHardware(struct wl_display *display)
{
    Q_D(WaylandEglClientBufferIntegration);

    const bool ignoreBindDisplay = !qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").isEmpty();

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qWarning("QtCompositor: Failed to initialize EGL display. No native platform interface available.");
        return;
    }

    d->egl_display = nativeInterface->nativeResourceForIntegration("EglDisplay");
    if (!d->egl_display) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not get EglDisplay for window.");
        return;
    }

    const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
    if ((!extensionString || !strstr(extensionString, "EGL_WL_bind_wayland_display")) && !ignoreBindDisplay) {
        qWarning("QtCompositor: Failed to initialize EGL display. There is no EGL_WL_bind_wayland_display extension.");
        return;
    }

    d->egl_bind_wayland_display = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
    d->egl_unbind_wayland_display = reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
    if ((!d->egl_bind_wayland_display || !d->egl_unbind_wayland_display) && !ignoreBindDisplay) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not find eglBindWaylandDisplayWL and eglUnbindWaylandDisplayWL.");
        return;
    }

    d->egl_query_wayland_buffer = reinterpret_cast<PFNEGLQUERYWAYLANDBUFFERWL_compat>(eglGetProcAddress("eglQueryWaylandBufferWL"));
    if (!d->egl_query_wayland_buffer) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not find eglQueryWaylandBufferWL.");
        return;
    }

    d->egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    d->egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!d->egl_create_image || !d->egl_destroy_image) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not find eglCreateImageKHR and eglDestroyImageKHR.");
        return;
    }

    if (d->egl_bind_wayland_display && d->egl_unbind_wayland_display) {
        d->display_bound = d->egl_bind_wayland_display(d->egl_display, display);
        if (!d->display_bound) {
            if (!ignoreBindDisplay) {
                qWarning("QtCompositor: Failed to initialize EGL display. Could not bind Wayland display.");
                return;
            } else {
                qWarning("QtCompositor: Could not bind Wayland display. Ignoring.");
            }
        }
    }

    d->funcs = new QEGLStreamConvenience;
    d->funcs->initialize(d->egl_display);

    d->valid = true;
}

void WaylandEglClientBufferIntegration::initializeBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);

    if (wl_shm_buffer_get(buffer))
        return;

    if (!buffer || d->buffers.contains(buffer))
        return;

    d->attach(buffer);
}

static QWaylandBufferRef::BufferFormatEgl formatFromEglFormat(EGLint format) {
    switch (format) {
    case EGL_TEXTURE_RGB:
        return QWaylandBufferRef::BufferFormatEgl_RGB;
    case EGL_TEXTURE_RGBA:
        return QWaylandBufferRef::BufferFormatEgl_RGBA;
    case EGL_TEXTURE_EXTERNAL_WL:
        return QWaylandBufferRef::BufferFormatEgl_EXTERNAL_OES;
    case EGL_TEXTURE_Y_UV_WL:
        return QWaylandBufferRef::BufferFormatEgl_Y_UV;
    case EGL_TEXTURE_Y_U_V_WL:
        return QWaylandBufferRef::BufferFormatEgl_Y_U_V;
    case EGL_TEXTURE_Y_XUXV_WL:
        return QWaylandBufferRef::BufferFormatEgl_Y_XUXV;
    }

    return QWaylandBufferRef::BufferFormatEgl_RGBA;
}

QWaylandBufferRef::BufferFormatEgl WaylandEglClientBufferIntegration::bufferFormat(wl_resource *buffer)
{
    Q_D(const WaylandEglClientBufferIntegration);
    return formatFromEglFormat(d->buffers.value(buffer).egl_format);
}

uint WaylandEglClientBufferIntegration::textureForBuffer(wl_resource *buffer, int plane)
{
    Q_UNUSED(plane)
    Q_D(WaylandEglClientBufferIntegration);
    if (!buffer)
        return 0;

    const BufferState state = d->buffers.value(buffer);
    return state.eglstream_texture;
}

void WaylandEglClientBufferIntegration::bindTextureToBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);
    d->bindBuffer(buffer);
}

// Update is only needed for the EGLStream path as that requires calling acquire
// on every frame. bindTextureToBuffer() is typically invoked only upon attach
// so that is insufficient.
void WaylandEglClientBufferIntegration::updateTextureForBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);
    if (!d->valid) {
        qWarning("QtCompositor: updateTextureForBuffer() failed");
        return;
    }

    if (!buffer)
        return;

    const BufferState state = d->buffers.value(buffer);

    if (state.egl_stream != EGL_NO_STREAM_KHR) {
        EGLint stream_state;
        d->funcs->query_stream(d->egl_display, state.egl_stream, EGL_STREAM_STATE_KHR, &stream_state);

        if (stream_state == EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR) {
            if (d->funcs->stream_consumer_acquire(d->egl_display, state.egl_stream) != EGL_TRUE)
                qWarning("%s:%d: eglStreamConsumerAcquireKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
        }
    }
}

QWaylandSurface::Origin WaylandEglClientBufferIntegration::origin(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    if (d->buffers.contains(buffer))
        return d->buffers[buffer].isYInverted ? QWaylandSurface::OriginTopLeft : QWaylandSurface::OriginBottomLeft;

#if defined(EGL_WAYLAND_Y_INVERTED_WL)
    EGLint isYInverted;
    EGLBoolean ret = EGL_FALSE;
    if (buffer)
        ret = d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WAYLAND_Y_INVERTED_WL, &isYInverted);
    // Yes, this looks strange, but the specification says that EGL_FALSE return
    // value (not supported) should be treated the same as EGL_TRUE return value
    // and EGL_TRUE in value.
    if (ret == EGL_FALSE || isYInverted == EGL_TRUE)
        return QWaylandSurface::OriginTopLeft;
    return QWaylandSurface::OriginBottomLeft;
#endif

    return QtWayland::ClientBufferIntegration::origin(buffer);
}


void *WaylandEglClientBufferIntegration::lockNativeBuffer(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    if (d->buffers.contains(buffer) && d->buffers[buffer].egl_stream != EGL_NO_STREAM_KHR)
        return 0;

    EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);
    return image;
}

void WaylandEglClientBufferIntegration::unlockNativeBuffer(void *native_buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    if (!native_buffer)
        return;

    EGLImageKHR image = static_cast<EGLImageKHR>(native_buffer);
    d->egl_destroy_image(d->egl_display, image);
}

QSize WaylandEglClientBufferIntegration::bufferSize(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    if (d->buffers.contains(buffer)) {
        return d->buffers[buffer].size;
    } else {
        int width, height;
        d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WIDTH, &width);
        d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_HEIGHT, &height);

        return QSize(width, height);
    }
}

QT_END_NAMESPACE
