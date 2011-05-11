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

#include "waylandsurface.h"

#include "wlcompositor.h"
#include "wlshmbuffer.h"

#include <QtCore/QDebug>

#include <wayland-server.h>

#include <linux/input.h>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "hardware_integration/graphicshardwareintegration.h"
#include <QtGui/QPlatformGLContext>
#endif

namespace Wayland {

class SurfacePrivate
{
public:
    SurfacePrivate(struct wl_client *client, Compositor *compositor)
        : client(client)
        , compositor(compositor)
        , needsMap(false)
        , textureCreatedForBuffer(false)
        , directRenderBuffer(0)
        , surfaceBuffer(0)
        , surfaceType(WaylandSurface::Invalid)
    {
#ifdef QT_COMPOSITOR_WAYLAND_GL
        texture_id = 0;
#endif
    }

    WaylandSurface::Type type() const {
        if (surfaceType == WaylandSurface::Invalid) {
            SurfacePrivate *that = const_cast<SurfacePrivate *>(this);
            if (qtSurface->handle() == compositor->directRenderSurface()) {
                that->surfaceType = WaylandSurface::Direct;
            } else if (surfaceBuffer && wl_buffer_is_shm(surfaceBuffer)) {
                that->surfaceType = WaylandSurface::Shm;
            } else if (surfaceBuffer){
                that->surfaceType = WaylandSurface::Texture;
            }
        }
        return surfaceType;
    }

    void attach(struct wl_buffer *buffer) {
        this->surfaceBuffer = buffer;
        surfaceType = WaylandSurface::Invalid;
        textureCreatedForBuffer = false;
    }

    inline struct wl_buffer *buffer() const { return surfaceBuffer; }

    struct wl_client *client;
    Compositor *compositor;
    WaylandSurface *qtSurface;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture_id;
#endif
    bool needsMap;
    bool textureCreatedForBuffer;
    wl_buffer *directRenderBuffer;

private:
    struct wl_buffer *surfaceBuffer;
    WaylandSurface::Type surfaceType;
};

void surface_destroy(struct wl_client *client, struct wl_surface *surface)
{
    wl_resource_destroy(&surface->resource, client, Compositor::currentTimeMsecs());
}

void surface_attach(struct wl_client *client, struct wl_surface *surface,
                    struct wl_buffer *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);

    wayland_cast<Surface *>(surface)->attach(buffer);
}

void surface_map_toplevel(struct wl_client *client,
                          struct wl_surface *surface)
{
    Q_UNUSED(client);
    printf("surface_map_toplevel: %p, %p\n", client, surface);

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
    printf("surface_map_transient: %p, %p\n", client, surface);
}

void surface_map_fullscreen(struct wl_client *client,
                       struct wl_surface *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    printf("surface_map_fullscreen: %p, %p\n", client, surface);
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
    d_ptr->qtSurface = new WaylandSurface(this);

}

Surface::~Surface()
{
    Q_D(Surface);
    d->compositor->surfaceDestroyed(this);
#ifdef QT_COMPOSITOR_WAYLAND_GL
    glDeleteTextures(1,&d->texture_id);
#endif
    delete d->qtSurface;
}

WaylandSurface::Type Surface::type() const
{
    Q_D(const Surface);
    return d->type();
}

bool Surface::isYInverted() const
{
    Q_D(const Surface);
    if (d->type() == WaylandSurface::Texture) {
        if (textureId()) {
            return d->compositor->graphicsHWIntegration()->isYInverted(d->buffer());
        }
    } else if (d->type() == WaylandSurface::Direct) {
        return d->compositor->graphicsHWIntegration()->isYInverted(d->buffer());
    }
    //shm surfaces are not flipped (in our "world")
    return false;
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);

#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (d->type() == WaylandSurface::Direct) {
        //should the texture be deleted here, or should we explicitly delete it
        //when going into direct mode...
        if (d->textureCreatedForBuffer) {
            glDeleteTextures(1,&d->texture_id);
            d->textureCreatedForBuffer = false;
        }
        if (d->compositor->graphicsHWIntegration()->postBuffer(d->directRenderBuffer))
                return;
    }
#endif

    if (d->needsMap) {
        QRect rect(0,0,d->buffer()->width,d->buffer()->height);
        emit d->qtSurface->mapped(rect);
        d->needsMap = false;
    }

    d->compositor->markSurfaceAsDirty(this);

    if (d->type() == WaylandSurface::Shm) {
        static_cast<ShmBuffer *>(d->buffer()->user_data)->damage();
    }

    emit d->qtSurface->damaged(rect);
}

QImage Surface::image() const
{
    Q_D(const Surface);
    if (d->type() == WaylandSurface::Shm) {
        ShmBuffer *shmBuffer = static_cast<ShmBuffer *>(d->buffer()->user_data);
        return shmBuffer->image();
    }
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint Surface::textureId() const
{
    Q_D(const Surface);
    if (!d->texture_id
            && d->type() == WaylandSurface::Texture
            && !d->textureCreatedForBuffer) {
        glDeleteTextures(1,&d->texture_id);
        Surface *that = const_cast<Surface *>(this);
        GraphicsHardwareIntegration *hwIntegration = d->compositor->graphicsHWIntegration();
        that->d_func()->texture_id = hwIntegration->createTextureFromBuffer(d->buffer());
        that->d_func()->textureCreatedForBuffer = true;
    }
    return d->texture_id;
}
#endif // QT_COMPOSITOR_WAYLAND_GL

void Surface::attach(struct wl_buffer *buffer)
{
    Q_D(Surface);
    d->attach(buffer);
}


void Surface::mapTopLevel()
{
    Q_D(Surface);
    if (!d->buffer())
        d->needsMap = true;
    else
        emit d->qtSurface->mapped(QRect(0,0,d->buffer()->width, d->buffer()->height));
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
        d->compositor->setPointerFocus(this, QPoint(x, y));
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
    d->compositor->setInputFocus(this);
}

}
