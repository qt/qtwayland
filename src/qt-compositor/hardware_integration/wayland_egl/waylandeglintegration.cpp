/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "waylandeglintegration.h"
#include "wayland_wrapper/wlcompositor.h"

#include <QtGui/QPlatformNativeInterface>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QPlatformScreen>
#include <QtGui/QWindow>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

GraphicsHardwareIntegration * GraphicsHardwareIntegration::createGraphicsHardwareIntegration(WaylandCompositor *compositor)
{
    return new WaylandEglIntegration(compositor);
}

class WaylandEglIntegrationPrivate
{
public:
    WaylandEglIntegrationPrivate()
        : egl_display(EGL_NO_DISPLAY), valid(false), flipperConnected(false)
    { }
    EGLDisplay egl_display;
    bool valid;
    bool flipperConnected;
};

WaylandEglIntegration::WaylandEglIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
    , d_ptr(new WaylandEglIntegrationPrivate)
{
}

void WaylandEglIntegration::initializeHardware(Wayland::Display *waylandDisplay)
{
    Q_D(WaylandEglIntegration);

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        d->egl_display = nativeInterface->nativeResourceForWindow("EglDisplay", m_compositor->window());
        if (d->egl_display) {
            const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
            if (extensionString && strstr(extensionString, "EGL_WL_bind_wayland_display")
                && eglBindWaylandDisplayWL(d->egl_display, waylandDisplay->handle()))
            {
                d->valid = true;
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
    EGLContext egl_context = nativeInterface->nativeResourceForContext("EglContext", context);

    EGLImageKHR image = eglCreateImageKHR(d->egl_display, egl_context,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);

    GLuint textureId;
    glGenTextures(1,&textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    eglDestroyImageKHR(d->egl_display, image);

    return textureId;
}

bool WaylandEglIntegration::setDirectRenderSurface(WaylandSurface *)
{
    Q_D(WaylandEglIntegration);

    QPlatformScreen *screen = QPlatformScreen::platformScreenForWindow(m_compositor->window());
    QPlatformScreenPageFlipper *flipper = screen ? screen->pageFlipper() : 0;
    if (flipper && !d->flipperConnected) {
        QObject::connect(flipper, SIGNAL(bufferReleased(void*)), m_compositor->handle(), SLOT(releaseBuffer(void*)));
        d->flipperConnected = true;
    }
    return flipper;
}


bool WaylandEglIntegration::postBuffer(struct wl_buffer *buffer)
{
    QPlatformScreen *screen = QPlatformScreen::platformScreenForWindow(m_compositor->window());
    QPlatformScreenPageFlipper *flipper = screen->pageFlipper();

    return flipper ? flipper->displayBuffer(buffer) : false;
}
