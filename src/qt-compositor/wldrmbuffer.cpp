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

#include "wldrmbuffer.h"

#include "wlcompositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <QtGui/private/qapplication_p.h>
#include <QtGui/QPlatformNativeInterface>
#include <QtCore/QDebug>

#include <xcb/dri2.h>
#include <xcb/xfixes.h>
#include <X11/Xlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

extern "C" {
#include <xf86drm.h>
}

namespace Wayland {

DrmBuffer::DrmBuffer(EGLImageKHR image, Compositor *compositor, struct wl_visual *visual, const QSize &size)
    : Buffer(compositor,visual,size)
    , m_image(image)
{

}

void DrmBuffer::attach(Surface *surface)
{
    Q_UNUSED(surface);
    printf("drmBuffer::attach\n");
}

void DrmBuffer::damage(Surface *surface, const QRect &rect)
{
    Q_UNUSED(surface);
    Q_UNUSED(rect);
    printf("drmBuffer::damage\n");

}


void authenticate(struct wl_client *client,
                             struct wl_drm *drm,
                             uint32_t id)
{
    qDebug() << "authenticate";
    Q_UNUSED(client);
    wl_object *object = (wl_object*)drm;
    DrmHandler *handler = wayland_cast<DrmHandler *>(object);
    handler->authenticate(id);
    wl_client_post_event(client, object, WL_DRM_AUTHENTICATED);
}

void create_buffer(struct wl_client *client,
                              struct wl_drm *drm,
                              uint32_t id,
                              uint32_t name,
                              int width,
                              int height,
                              uint32_t stride,
                              struct wl_visual *visual)
{
    wl_object *object = (wl_object*)drm;
    DrmHandler *handler = wayland_cast<DrmHandler *>(object);
    handler->createBuffer(client,id,name,QSize(width,height),stride,visual);
}

void post_drm_device(struct wl_client *client, struct wl_object *global)
{
    //Lighthouse apis needs to bleed this, think I need to create a native interface abstraction
    wl_client_post_event(client, global, WL_DRM_DEVICE,"/dev/dri/card0");
}

DrmHandler::DrmHandler(Compositor *compositor)
    :m_compositor(compositor)
{
    QPlatformNativeInterface *nativeInterface = QApplicationPrivate::platformIntegration()->nativeInterface();
    m_connection = static_cast<xcb_connection_t *>(nativeInterface->nativeResourceForWidget("Connection",0));
    xcb_screen_t *screen = static_cast<xcb_screen_t *>(nativeInterface->nativeResourceForWidget("Screen",0));
    m_root_window = screen->root;
    m_egl_display = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWidget("EglDisplay",0));

    initializeDrm();
}

void DrmHandler::initializeDrm()
{
}

void DrmHandler::authenticate(uint32_t id)
{
    xcb_dri2_authenticate_cookie_t authenticateCoockie = xcb_dri2_authenticate_unchecked(m_connection,m_root_window,id);
    xcb_dri2_authenticate_reply_t *authenticate = xcb_dri2_authenticate_reply(m_connection,authenticateCoockie,NULL);

    if (!authenticate || !authenticate->authenticated) {
        fprintf(stderr,"Failed to authenticate drm :(\n");
    }

    free(authenticate);
}

void DrmHandler::createBuffer(wl_client *client, uint32_t id, uint32_t name, const QSize &size, uint32_t stride, wl_visual *visual)
{
    EGLImageKHR image;
    EGLint attribs[] = {
            EGL_WIDTH,                      size.width(),
            EGL_HEIGHT,                     size.height(),
            EGL_DRM_BUFFER_STRIDE_MESA,     stride /4,
            EGL_DRM_BUFFER_FORMAT_MESA,     EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
            EGL_NONE
    };

    image = eglCreateImageKHR(m_egl_display,
                              EGL_NO_CONTEXT,
                              EGL_DRM_BUFFER_MESA,
                              (EGLClientBuffer) name, attribs);

    DrmBuffer *buffer = new DrmBuffer(image,m_compositor,visual,size);

    addClientResource(client,&buffer->base()->resource,id,&wl_buffer_interface,&drm_interface,0);
}

}
