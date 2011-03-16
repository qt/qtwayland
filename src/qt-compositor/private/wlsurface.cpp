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

#include "wlsurface.h"

#include "../waylandsurface.h"

#include "wlcompositor.h"
#include "wlshmbuffer.h"

#include <QtCore/QDebug>

#include <wayland-server.h>

#include <linux/input.h>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "../graphicshardwareintegration.cpp"
#include <QtGui/QPlatformGLContext>
#endif

namespace Wayland {

class SurfacePrivate
{
public:
    SurfacePrivate(struct wl_client *client, Compositor *compositor)
        : client(client)
        , compositor(compositor)
    {
        type = WaylandSurface::Invalid;
#ifdef QT_COMPOSITOR_WAYLAND_GL
        texture_id = 0;
#endif
    }

    WaylandSurface::Type type;
    QRect rect;
    struct wl_client *client;
    Compositor *compositor;
    WaylandSurface *qtSurface;

    ShmBuffer *shm_buffer;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture_id;
#endif
};



void surface_destroy(struct wl_client *client, struct wl_surface *surface)
{
    wl_resource_destroy(&surface->resource, client);
}

void surface_attach(struct wl_client *client, struct wl_surface *surface,
                    struct wl_buffer *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);

    if (buffer->attach) {
        wayland_cast<Surface *>(surface)->attachShm(wayland_cast<ShmBuffer *>(buffer));
    }
#ifdef QT_COMPOSITOR_WAYLAND_GL
    else {
        wayland_cast<Surface *>(surface)->attachHWBuffer(buffer);
    }
#endif //QT_COMPOSITOR_WAYLAND_EGL
}

void surface_map_toplevel(struct wl_client *client,
                          struct wl_surface *surface)
{
    Q_UNUSED(client);
    printf("surface_map_toplevel: %p, %p", client, surface);

    wayland_cast<Surface *>(surface)->mapTopLevel();
}

void surface_map_transient(struct wl_client *client,
                      struct wl_surface *surface,
                      struct wl_surface *parent,
                      int x,
                      int y,
                      uint32_t flags)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(parent);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(flags);
    printf("surface_map_transient: %p, %p", client, surface);
}

void surface_map_fullscreen(struct wl_client *client,
                       struct wl_surface *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    printf("surface_map_fullscreen: %p, %p", client, surface);
}

void surface_damage(struct wl_client *client, struct wl_surface *surface,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    wayland_cast<Surface *>(surface)->damage(QRect(x, y, width, height));
}

Surface::Surface(struct wl_client *client, Compositor *compositor)
    : d_ptr(new SurfacePrivate(client,compositor))
{
    base()->client = client;
    wl_list_init(&base()->destroy_listener_list);
    d_ptr->qtSurface = new WaylandSurface(this);

}

Surface::~Surface()
{
    Q_D(Surface);
    d->compositor->surfaceDestroyed(this);
    delete d->qtSurface;
}

WaylandSurface::Type Surface::type() const
{
    Q_D(const Surface);
    return d->type;
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);
    emit d->qtSurface->damaged(rect);
}

QImage Surface::image() const
{
    Q_D(const Surface);
    if (d->type == WaylandSurface::Shm) {
        return d->shm_buffer->image();
    }
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL

void Surface::attachHWBuffer(struct wl_buffer *buffer)
{
    Q_D(Surface);
    d->type = WaylandSurface::Texture;

    //make current for the topLevel. We could have used the eglContext,
    //but then we would need to handle eglSurfaces as well.
    d->compositor->topLevelWidget()->platformWindow()->glContext()->makeCurrent();

    glDeleteTextures(1,&d->texture_id);

    d->texture_id = d->compositor->graphicsHWIntegration()->createTextureFromBuffer(buffer);
    emit d->qtSurface->mapped(QRect(QPoint(), QSize(buffer->width, buffer->height)));
}

GLuint Surface::textureId() const
{
    Q_D(const Surface);
    return d->texture_id;
}
#endif // QT_COMPOSITOR_WAYLAND_GL

void Surface::attachShm(Wayland::ShmBuffer *shm_buffer)
{
    Q_D(Surface);
    d->shm_buffer = shm_buffer;
    d->type = WaylandSurface::Shm;
    emit d->qtSurface->mapped(QRect(QPoint(), shm_buffer->size()));
}


void Surface::mapTopLevel()
{
    Q_D(Surface);
    d->rect = QRect(0, 0, 200, 200);
}

WaylandSurface * Surface::handle() const
{
    Q_D(const Surface);
    return d->qtSurface;
}

uint32_t toWaylandButton(Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:
        return BTN_LEFT;
    case Qt::RightButton:
        return BTN_RIGHT;
    default:
        return BTN_MIDDLE;
    }
}

void Surface::sendMousePressEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(Surface);
    sendMouseMoveEvent(x, y);
    if (d->client) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                             WL_INPUT_DEVICE_BUTTON, time, toWaylandButton(button), 1);
    }
}

void Surface::sendMouseReleaseEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(Surface);
    sendMouseMoveEvent(x, y);
    if (d->client) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                             WL_INPUT_DEVICE_BUTTON, time, toWaylandButton(button), 0);
    }
}

void Surface::sendMouseMoveEvent(int x, int y)
{
    Q_D(Surface);
    if (d->client) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_input_device_set_pointer_focus(d->compositor->defaultInputDevice(),
            base(), time, x, y, x, y);
        wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
             WL_INPUT_DEVICE_MOTION, time, x, y, x, y);
    }
}

void Surface::sendKeyPressEvent(uint code)
{
    Q_D(Surface);
    if (d->compositor->defaultInputDevice()->keyboard_focus != NULL) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                             WL_INPUT_DEVICE_KEY, time, code - 8, 1);
    }

}

void Surface::sendKeyReleaseEvent(uint code)
{
    Q_D(Surface);
    if (d->compositor->defaultInputDevice()->keyboard_focus != NULL) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                             WL_INPUT_DEVICE_KEY, time, code - 8, 0);
    }
}

void Surface::setInputFocus()
{
    Q_D(Surface);
    ulong time = d->compositor->currentTimeMsecs();

    wl_input_device_set_keyboard_focus(d->compositor->defaultInputDevice(), base(), time);
    wl_input_device_set_pointer_focus(d->compositor->defaultInputDevice(), base(), time, 0, 0, 0, 0);
}

}
