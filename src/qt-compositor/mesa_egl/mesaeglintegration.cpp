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

#include "mesaeglintegration.h"

#include <QtGui/QPlatformNativeInterface>
#include <QtGui/private/qapplication_p.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

MesaEglIntegration::MesaEglIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
{
}

void MesaEglIntegration::intializeHardware(wl_display *waylandDisplay)
{
    QPlatformNativeInterface *nativeInterface = QApplicationPrivate::platformIntegration()->nativeInterface();
    if (nativeInterface) {
        EGLDisplay m_egl_display = nativeInterface->nativeResourceForWidget("EglDisplay",0);
        if (m_egl_display) {
            eglBindWaylandDisplayWL(m_egl_display,waylandDisplay);
        } else {
            fprintf(stderr, "Failed to initialize egl display");
        }
    }
}

void MesaEglIntegration::bindBufferToTexture(wl_buffer *buffer, GLuint textureId)
{
    QPlatformNativeInterface *nativeInterface = QApplicationPrivate::platformIntegration()->nativeInterface();
    EGLDisplay eglDisplay = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWidget("EglDisplay",m_compositor->topLevelWidget()));
    EGLContext eglContext = static_cast<EGLContext>(nativeInterface->nativeResourceForWidget("EglContext",m_compositor->topLevelWidget()));
    Q_ASSERT(eglDisplay);
    Q_ASSERT(eglContext);

    EGLImageKHR image = eglCreateImageKHR(eglDisplay, eglContext,
                                          EGL_WAYLAND_BUFFER_WL,
                                          buffer, NULL);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    eglDestroyImageKHR(eglDisplay, image);


}
