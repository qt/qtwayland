/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandeglintegration.h"

#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlsurface_p.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <qpa/qplatformscreen.h>
#include <QtGui/QWindow>
#include <QtCore/QPointer>

#include <QDebug>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef EGL_WL_bind_wayland_display
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL) (EGLDisplay dpy, struct wl_buffer *buffer, EGLint attribute, EGLint *value);
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

class WaylandEglIntegrationPrivate
{
public:
    WaylandEglIntegrationPrivate()
        : egl_display(EGL_NO_DISPLAY)
        , valid(false)
        , display_bound(false)
        , flipperConnected(false)
        , egl_bind_wayland_display(0)
        , egl_unbind_wayland_display(0)
        , egl_query_wayland_buffer(0)
        , egl_create_image(0)
        , egl_destroy_image(0)
        , gl_egl_image_target_texture_2d(0)
    { }
    EGLDisplay egl_display;
    bool valid;
    bool display_bound;
    bool flipperConnected;
#ifdef EGL_WL_request_client_buffer_format
    QPointer<WaylandSurface> directRenderSurface;
#endif
    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display;
    PFNEGLQUERYWAYLANDBUFFERWL egl_query_wayland_buffer;

    PFNEGLCREATEIMAGEKHRPROC egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d;
};

WaylandEglIntegration::WaylandEglIntegration()
    : QWaylandGraphicsHardwareIntegration()
    , d_ptr(new WaylandEglIntegrationPrivate)
{
}

void WaylandEglIntegration::initializeHardware(QtWayland::Display *waylandDisplay)
{
    Q_D(WaylandEglIntegration);

    const bool ignoreBindDisplay = !qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").isEmpty();

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qWarning("Failed to initialize egl display. No native platform interface available.\n");
        return;
    }

    d->egl_display = nativeInterface->nativeResourceForWindow("EglDisplay", m_compositor->window());
    if (!d->egl_display) {
        qWarning("Failed to initialize egl display. Could not get EglDisplay for window.\n");
        return;
    }

    const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
    if ((!extensionString || !strstr(extensionString, "EGL_WL_bind_wayland_display")) && !ignoreBindDisplay) {
        qWarning("Failed to initialize egl display. There is no EGL_WL_bind_wayland_display extension.\n");
        return;
    }

    d->egl_bind_wayland_display = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
    d->egl_unbind_wayland_display = reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
    if (!d->egl_bind_wayland_display || !d->egl_unbind_wayland_display && !ignoreBindDisplay) {
        qWarning("Failed to initialize egl display. Could not find eglBindWaylandDisplayWL and eglUnbindWaylandDisplayWL.\n");
        return;
    }

    d->egl_query_wayland_buffer = reinterpret_cast<PFNEGLQUERYWAYLANDBUFFERWL>(eglGetProcAddress("eglQueryWaylandBufferWL"));
    if (!d->egl_query_wayland_buffer) {
        qWarning("Failed to initialize egl display. Could not find eglQueryWaylandBufferWL.\n");
        return;
    }

    d->egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    d->egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!d->egl_create_image || !d->egl_destroy_image) {
        qWarning("Failed to initialize egl display. Could not find eglCreateImageKHR and eglDestroyImageKHR.\n");
        return;
    }

    d->gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!d->gl_egl_image_target_texture_2d) {
        qWarning("Failed to initialize egl display. Could not find glEGLImageTargetTexture2DOES.\n");
        return;
    }

    if (d->egl_bind_wayland_display && d->egl_unbind_wayland_display) {
        d->display_bound = d->egl_bind_wayland_display(d->egl_display, waylandDisplay->handle());
        if (!d->display_bound || ignoreBindDisplay) {
            qWarning("Failed to initialize egl display. Could not bind Wayland display.\n");
            return;
        }
    }

    d->valid = true;

    qWarning("EGL Wayland extension successfully initialized.%s\n", !d->display_bound ? " eglBindWaylandDisplayWL ignored" : "");
}

GLuint WaylandEglIntegration::createTextureFromBuffer(struct ::wl_resource *buffer, QOpenGLContext *)
{
    Q_D(WaylandEglIntegration);
    if (!d->valid) {
        qWarning("createTextureFromBuffer() failed\n");
        return 0;
    }

    EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);

    GLuint textureId;
    glGenTextures(1,&textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);

    d->gl_egl_image_target_texture_2d(GL_TEXTURE_2D, image);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    d->egl_destroy_image(d->egl_display, image);

    return textureId;
}

bool WaylandEglIntegration::isYInverted(struct ::wl_resource *buffer) const
{
#ifdef EGL_WL_request_client_buffer_format
    return eglGetBufferYInvertedWL(buffer);
#else
    return QWaylandGraphicsHardwareIntegration::isYInverted(buffer);
#endif
}


bool WaylandEglIntegration::setDirectRenderSurface(QWaylandSurface *surface)
{
    Q_D(WaylandEglIntegration);

    QPlatformScreen *screen = QPlatformScreen::platformScreenForWindow(m_compositor->window());
    QPlatformScreenPageFlipper *flipper = screen ? screen->pageFlipper() : 0;
    if (flipper && !d->flipperConnected) {
        QObject::connect(flipper, SIGNAL(bufferReleased(QPlatformScreenBuffer*)), m_compositor->handle(), SLOT(releaseBuffer(QPlatformScreenBuffer*)));
        d->flipperConnected = true;
    }
#ifdef EGL_WL_request_client_buffer_format
    int buffer_format = surface ? EGL_SCANOUT_FORMAT_WL : EGL_RENDER_FORMAT_WL;
    struct wl_client *client = 0;
    if (surface) {
        client = surface->handle()->base()->resource.client;
    } else {
        WaylandSurface *oldSurface = d->directRenderSurface.data();
        if (oldSurface)
            client = oldSurface->handle()->base()->resource.client;
    }
    if (client)
        eglRequestClientBufferFormatWL(d->egl_display, client, buffer_format);
    d->directRenderSurface = surface;
#else
    Q_UNUSED(surface);
#endif
    return flipper;
}

void *WaylandEglIntegration::lockNativeBuffer(struct ::wl_resource *buffer, QOpenGLContext *) const
{
    Q_D(const WaylandEglIntegration);

    EGLImageKHR image = d->egl_create_image(d->egl_display, EGL_NO_CONTEXT,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);
    return image;
}

void WaylandEglIntegration::unlockNativeBuffer(void *native_buffer, QOpenGLContext *) const
{
    Q_D(const WaylandEglIntegration);
    EGLImageKHR image = static_cast<EGLImageKHR>(native_buffer);

    d->egl_destroy_image(d->egl_display, image);
}

QSize WaylandEglIntegration::bufferSize(struct ::wl_resource *buffer) const
{
    Q_D(const WaylandEglIntegration);

    int width, height;
    d->egl_query_wayland_buffer(d->egl_display, reinterpret_cast<struct ::wl_buffer *>(buffer), EGL_WIDTH, &width);
    d->egl_query_wayland_buffer(d->egl_display, reinterpret_cast<struct ::wl_buffer *>(buffer), EGL_HEIGHT, &height);

    return QSize(width, height);
}

QT_END_NAMESPACE
