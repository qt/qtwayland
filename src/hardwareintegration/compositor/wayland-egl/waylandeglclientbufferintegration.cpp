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
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

#ifndef EGL_WAYLAND_BUFFER_WL
#define EGL_WAYLAND_BUFFER_WL           0x31D5
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
    BufferState()
        : gl_texture(0)
        , gl_texture_target(GL_TEXTURE_2D)
        , egl_stream(EGL_NO_STREAM_KHR)
        , isYInverted(true)
        {}

    GLuint gl_texture;
    GLenum gl_texture_target;
    EGLStreamKHR egl_stream;
    bool isYInverted;
    QSize size;
};

struct buffer_destroy_listener
{
    struct wl_listener listener;
    class WaylandEglClientBufferIntegrationPrivate *d;
};

class WaylandEglClientBufferIntegrationPrivate
{
public:
    WaylandEglClientBufferIntegrationPrivate()
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
    {
    }

    static void destroy_listener_callback(wl_listener *listener, void *data) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);

        buffer_destroy_listener *destroy_listener = reinterpret_cast<buffer_destroy_listener *>(listener);
        WaylandEglClientBufferIntegrationPrivate *self = destroy_listener->d;
        struct ::wl_resource *buffer = static_cast<struct ::wl_resource *>(data);

        wl_list_remove(&listener->link);
        delete listener;

        if (!self->buffers.contains(buffer))
            return;

        Q_ASSERT(self);
        Q_ASSERT(buffer);

        BufferState state = self->buffers.take(buffer);

        if (state.gl_texture != 0)
            glDeleteTextures(1, &state.gl_texture);

        if (state.egl_stream != EGL_NO_STREAM_KHR)
            self->funcs->destroy_stream(self->egl_display, state.egl_stream);
    }

    void create_destroy_listener(struct ::wl_resource *buffer) {
        buffer_destroy_listener *newListener = new buffer_destroy_listener;
        newListener->d = this;
        newListener->listener.notify = destroy_listener_callback;

        wl_signal_add(&buffer->destroy_signal, &newListener->listener);
    }

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

static GLuint make_texture(GLenum target)
{
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(target, texture);

    return texture;
}

static void set_texture_params(GLenum target)
{
    glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void WaylandEglClientBufferIntegration::initializeBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);

    if (wl_shm_buffer_get(buffer))
        return;

    if (!buffer || d->buffers.contains(buffer))
        return;

    d->create_destroy_listener(buffer);
}

int WaylandEglClientBufferIntegration::textureTargetForBuffer(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglClientBufferIntegration);

    return d->buffers.value(buffer).gl_texture_target;
}

void WaylandEglClientBufferIntegration::bindTextureToBuffer(struct ::wl_resource *buffer)
{
    Q_D(WaylandEglClientBufferIntegration);
    if (!d->valid) {
        qWarning("QtCompositor: bindTextureToBuffer() failed");
        return;
    }

    if (!buffer)
        return;

    BufferState state = d->buffers.value(buffer);

    if (state.egl_stream != EGL_NO_STREAM_KHR) {
        d->funcs->stream_consumer_acquire(d->egl_display, state.egl_stream);
    } else {
        Q_ASSERT(QOpenGLContext::currentContext());

        EGLint format;
        EGLNativeFileDescriptorKHR streamFd = EGL_NO_FILE_DESCRIPTOR_KHR;

        if (d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_TEXTURE_FORMAT, &format)) {
            // Resolving GL functions may need a context current, so do it only here.
            if (!d->gl_egl_image_target_texture_2d)
                d->gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

            if (!d->gl_egl_image_target_texture_2d) {
                qWarning("QtCompositor: bindTextureToBuffer() failed. Could not find glEGLImageTargetTexture2DOES.");
                return;
            }

            EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                                    EGL_WAYLAND_BUFFER_WL,
                                                    buffer, NULL);

            d->gl_egl_image_target_texture_2d(GL_TEXTURE_2D, image);
            set_texture_params(GL_TEXTURE_2D);
            d->egl_destroy_image(d->egl_display, image);
        } else if (d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WAYLAND_BUFFER_WL, &streamFd)) {
            state.egl_stream = d->funcs->create_stream_from_file_descriptor(d->egl_display, streamFd);
            close(streamFd);

            if (state.egl_stream == EGL_NO_STREAM_KHR) {
                qWarning("%s:%d: eglCreateStreamFromFileDescriptorKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
                return;
            }

            state.isYInverted = false;
            state.gl_texture_target = GL_TEXTURE_EXTERNAL_OES;
            state.gl_texture = make_texture(state.gl_texture_target);
            set_texture_params(state.gl_texture_target);

            if (d->funcs->stream_consumer_gltexture(d->egl_display, state.egl_stream) != EGL_TRUE)
                qWarning("%s:%d: eglStreamConsumerGLTextureExternalKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
        }

        EGLint width, height;
        d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WIDTH, &width);
        d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_HEIGHT, &height);
        state.size = QSize(width, height);

#if defined(EGL_WAYLAND_Y_INVERTED_WL)
        EGLint isYInverted;
        EGLBoolean ret = d->egl_query_wayland_buffer(d->egl_display, buffer, EGL_WAYLAND_Y_INVERTED_WL, &isYInverted);
        // Yes, this looks strange, but the specification says that EGL_FALSE return
        // value (not supported) should be treated the same as EGL_TRUE return value
        // and EGL_TRUE in value.
        state.isYInverted = (ret == EGL_FALSE || isYInverted == EGL_TRUE);
#endif

        d->buffers[buffer] = state;
    }
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

    if (state.egl_stream != EGL_NO_STREAM_KHR)
        d->funcs->stream_consumer_acquire(d->egl_display, state.egl_stream);
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
