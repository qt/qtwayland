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

#include "wlcompositor.h"
#include "wlshmbuffer.h"
#include <QtCore/QDebug>

namespace Wayland {

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

    wayland_cast<Surface *>(surface)->attach(wayland_cast<ShmBuffer *>(buffer));
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
    : m_client(client)
    , m_compositor(compositor)
{
    base()->client = client;
    wl_list_init(&base()->destroy_listener_list);

    current.buffer = 0;
    staged.buffer = 0;
}

Surface::~Surface()
{
    m_compositor->surfaceDestroyed(this);
}

void Surface::damage(const QRect &rect)
{
    m_compositor->surfaceDamaged(this, rect);
}

QImage Surface::image() const
{
    if (current.buffer && current.buffer->type() == Buffer::Shm) {
        return static_cast<ShmBuffer *>(current.buffer)->image();
    }
    return QImage();
}

void Surface::attach(Buffer *buffer)
{
    current.buffer = staged.buffer = buffer;
    m_compositor->surfaceResized(this, buffer->size());
}

}
