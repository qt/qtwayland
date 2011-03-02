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

#ifndef WL_DRMBUFFER_H
#define WL_DRMBUFFER_H

#include "wlbuffer.h"

#include <QtGui/QPlatformIntegration>

#include <wayland-server.h>

#include <xcb/xcb.h>

#define MESA_EGL_NO_X11_HEADERS
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

//class Display;

namespace Wayland {

class Compositor;
struct Display;
class DrmBuffer : public Buffer
{
public:
    DrmBuffer(EGLImageKHR image, Compositor *compositor, struct wl_visual *visual, const QSize &size);

    Buffer::BufferType type() const {return Buffer::Egl;}

    void attach(Surface *surface);
    void damage(Surface *surface, const QRect &rect);

private:
    EGLImageKHR m_image;
};

class DrmHandler : public Object<struct wl_object>
{
public:
    DrmHandler(Compositor *compositor);
    void initializeDrm();

    void authenticate(uint32_t id);
    void createBuffer(struct wl_client *client, uint32_t id, uint32_t name,
                      const QSize &size, uint32_t stride, struct wl_visual *visual);
private:

    xcb_connection_t *m_connection;
    xcb_window_t m_root_window;
    EGLDisplay m_egl_display;

    Compositor *m_compositor;
};

void authenticate(struct wl_client *client,
                             struct wl_drm *drm,
                             uint32_t id);
void create_buffer(struct wl_client *client,
                              struct wl_drm *drm,
                              uint32_t id,
                              uint32_t name,
                              int width,
                              int height,
                              uint32_t stride,
                              struct wl_visual *visual);

void post_drm_device(struct wl_client *client, struct wl_object *global);

const struct wl_drm_interface drm_interface = {
    authenticate,
    create_buffer
};


}

#endif // WL_DRMBUFFER_H
