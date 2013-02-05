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
#endif

#ifndef EGL_KHR_image
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);
#endif

QT_USE_NAMESPACE

class WaylandEglIntegrationPrivate
{
public:
    WaylandEglIntegrationPrivate()
        : egl_display(EGL_NO_DISPLAY)
        , valid(false)
        , flipperConnected(false)
        , egl_bind_wayland_display(0)
        , egl_unbind_wayland_display(0)
        , egl_create_image(0)
        , egl_destroy_image(0)
        , gl_egl_image_target_texture_2d(0)
    { }
    EGLDisplay egl_display;
    bool valid;
    bool flipperConnected;
#ifdef EGL_WL_request_client_buffer_format
    QPointer<WaylandSurface> directRenderSurface;
#endif
    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display;

    PFNEGLCREATEIMAGEKHRPROC egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d;

    QPlatformNativeInterface::NativeResourceForContextFunction get_egl_context;
};

WaylandEglIntegration::WaylandEglIntegration()
    : QWaylandGraphicsHardwareIntegration()
    , d_ptr(new WaylandEglIntegrationPrivate)
{
}

void WaylandEglIntegration::initializeHardware(QtWayland::Display *waylandDisplay)
{
    Q_D(WaylandEglIntegration);

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        d->egl_display = nativeInterface->nativeResourceForWindow("EglDisplay", m_compositor->window());
        if (d->egl_display) {
            const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
            if (extensionString && strstr(extensionString, "EGL_WL_bind_wayland_display"))
            {
                d->get_egl_context = nativeInterface->nativeResourceFunctionForContext("get_egl_context");
                if (!d->get_egl_context) {
                    qWarning("Failed to retrieve the get_egl_context function");
                }
                d->egl_bind_wayland_display =
                        reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
                d->egl_unbind_wayland_display =
                        reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
                d->egl_create_image =
                        reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
                d->egl_destroy_image =
                        reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
                d->gl_egl_image_target_texture_2d =
                        reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

                if (d->egl_bind_wayland_display
                        && d->get_egl_context
                        && d->egl_unbind_wayland_display
                        && d->egl_create_image
                        && d->egl_destroy_image
                        && d->gl_egl_image_target_texture_2d) {
                    if (d->egl_bind_wayland_display(d->egl_display, waylandDisplay->handle())) {
                        d->valid = true;
                    }
                }
            }
        }

        if (!d->valid)
            qWarning("Failed to initialize egl display\n");
    }
}

GLuint WaylandEglIntegration::createTextureFromBuffer(wl_buffer *buffer, QOpenGLContext *context)
{
    Q_D(WaylandEglIntegration);
    if (!d->valid) {
        qWarning("createTextureFromBuffer() failed\n");
        return 0;
    }

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    EGLContext egl_context = d->get_egl_context(context);

    EGLImageKHR image = d->egl_create_image(d->egl_display, egl_context,
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

bool WaylandEglIntegration::isYInverted(struct wl_buffer *buffer) const
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
        QObject::connect(flipper, SIGNAL(bufferReleased(void*)), m_compositor->handle(), SLOT(releaseBuffer(void*)));
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

void *WaylandEglIntegration::lockNativeBuffer(struct wl_buffer *buffer, QOpenGLContext *context) const
{
    Q_D(const WaylandEglIntegration);

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    EGLContext egl_context = d->get_egl_context(context);

    EGLImageKHR image = d->egl_create_image(d->egl_display, egl_context,
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

