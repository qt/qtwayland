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

#ifdef Q_OS_LINUX
#include <linux/input.h>
#endif

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "hardware_integration/graphicshardwareintegration.h"
#include <QtGui/QPlatformOpenGLContext>
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "waylandwindowmanagerintegration.h"
#endif

namespace Wayland {

class SurfacePrivate
{
public:
    SurfacePrivate(struct wl_client *client, Compositor *compositor)
        : client(client)
        , compositor(compositor)
        , textureCreatedForBuffer(false)
        , directRenderBuffer(0)
        , processId(0)
        , previousBuffer(0)
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
        bool emitMap = !surfaceBuffer;
        if (surfaceBuffer && ! textureCreatedForBuffer) {
            qWarning() << "### WaylandSurface::attach() releasing undisplayed buffer ###";
             wl_client_post_event(client,&surfaceBuffer->resource.object,WL_BUFFER_RELEASE);
        }
        surfaceBuffer = buffer;
        surfaceType = WaylandSurface::Invalid;
        textureCreatedForBuffer = false;
        if (emitMap) {
            qtSurface->mapped(QSize(surfaceBuffer->width,surfaceBuffer->height));
        }
    }

    inline struct wl_buffer *buffer() const { return surfaceBuffer; }

    struct wl_client *client;
    Compositor *compositor;
    WaylandSurface *qtSurface;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture_id;
#endif
    bool textureCreatedForBuffer;
    struct wl_buffer *directRenderBuffer;
    qint64 processId;
    QByteArray authenticationToken;
    QVariantMap windowProperties;

    QPoint lastMousePos;

    struct wl_buffer *previousBuffer;
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
    if (d->compositor->graphicsHWIntegration())
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
#ifdef QT_COMPOSITOR_WAYLAND_GL
    Q_D(const Surface);

    if (d->compositor->graphicsHWIntegration()) {
        if (d->type() == WaylandSurface::Texture) {
            if (textureId()) {
                return d->compositor->graphicsHWIntegration()->isYInverted(d->buffer());
            }
        } else if (d->type() == WaylandSurface::Direct) {
            return d->compositor->graphicsHWIntegration()->isYInverted(d->buffer());
        }
    }
#endif
    //shm surfaces are not flipped (in our "world")
    return false;
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);

#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (d->compositor->graphicsHWIntegration() && d->type() == WaylandSurface::Direct) {
        //should the texture be deleted here, or should we explicitly delete it
        //when going into direct mode...
        if (d->textureCreatedForBuffer) {
            glDeleteTextures(1,&d->texture_id);
            d->textureCreatedForBuffer = false;
        }
        if (d->compositor->graphicsHWIntegration()->postBuffer(d->buffer())) {
            if (d->previousBuffer) {
                wl_client_post_event(d->client,&d->previousBuffer->resource.object,WL_BUFFER_RELEASE);
            }
            d->previousBuffer = d->buffer();
            return;
        }
    }
#endif

    d->compositor->markSurfaceAsDirty(this);

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
    if (d->compositor->graphicsHWIntegration() && d->type() == WaylandSurface::Texture
         && !d->textureCreatedForBuffer) {
        glDeleteTextures(1,&d->texture_id);
        if (d->previousBuffer) {
            wl_client_post_event(d->client,&d->previousBuffer->resource.object,WL_BUFFER_RELEASE);
        }
        Surface *that = const_cast<Surface *>(this);
        GraphicsHardwareIntegration *hwIntegration = d->compositor->graphicsHWIntegration();
        that->d_func()->previousBuffer = d->buffer();
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

WaylandSurface * Surface::handle() const
{
    Q_D(const Surface);
    return d->qtSurface;
}

wl_client *Surface::clientHandle() const
{
    Q_D(const Surface);
    return d->client;
}

qint64 Surface::processId() const
{
    Q_D(const Surface);
    return d->processId;
}

void Surface::setProcessId(qint64 processId)
{
    Q_D(Surface);
    d->processId = processId;
}

QByteArray Surface::authenticationToken() const
{
    Q_D(const Surface);
    return WindowManagerServerIntegration::instance()->managedClient(d->client)->authenticationToken();
}

QVariantMap Surface::windowProperties() const
{
    Q_D(const Surface);
    return d->windowProperties;
}

QVariant Surface::windowProperty(const QString &propertyName) const
{
    Q_D(const Surface);
    QVariantMap props = d->windowProperties;
    return props.value(propertyName);
}

void Surface::setWindowProperty(const QString &name, const QVariant &value, bool writeUpdateToClient)
{
    Q_D(Surface);
    d->windowProperties.insert(name, value);
    if (writeUpdateToClient)
        WindowManagerServerIntegration::instance()->setWindowProperty(d->client, base(), name, value);
}

uint32_t toWaylandButton(Qt::MouseButton button)
{
#ifndef BTN_LEFT
uint32_t BTN_LEFT = 0x110;
uint32_t BTN_RIGHT = 0x111;
uint32_t BTN_MIDDLE = 0x112;
#endif
    switch (button) {
    case Qt::LeftButton:
        return BTN_LEFT;
    case Qt::RightButton:
        return BTN_RIGHT;
    default:
        return BTN_MIDDLE;
    }

}

QPoint Surface::lastMousePos() const
{
    Q_D(const Surface);
    return d->lastMousePos;
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
        d->lastMousePos = QPoint(x, y);
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

void Surface::sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state)
{
    Q_D(Surface);
    uint32_t time = d->compositor->currentTimeMsecs();
    struct wl_client *client = d->client;
    struct wl_object *dev = &d->compositor->defaultInputDevice()->object;
    switch (state) {
    case Qt::TouchPointPressed:
        wl_client_post_event(client, dev, WL_INPUT_DEVICE_TOUCH_DOWN, time, id, x, y);
        break;
    case Qt::TouchPointMoved:
        wl_client_post_event(client, dev, WL_INPUT_DEVICE_TOUCH_MOTION, time, id, x, y);
        break;
    case Qt::TouchPointReleased:
        wl_client_post_event(client, dev, WL_INPUT_DEVICE_TOUCH_UP, time, id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

void Surface::sendTouchFrameEvent()
{
    Q_D(Surface);
    wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                         WL_INPUT_DEVICE_TOUCH_FRAME);
}

void Surface::sendTouchCancelEvent()
{
    Q_D(Surface);
    wl_client_post_event(d->client, &d->compositor->defaultInputDevice()->object,
                         WL_INPUT_DEVICE_TOUCH_CANCEL);
}

void Surface::frameFinished()
{
    Q_D(Surface);
    d->compositor->frameFinished(this);
}

void Surface::setInputFocus()
{
    Q_D(Surface);
    d->compositor->setInputFocus(this);
}

void Surface::sendOnScreenVisibilityChange(bool visible)
{
#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    Q_D(Surface);
    WindowManagerServerIntegration::instance()->setVisibilityOnScreen(d->client, visible);
#endif
}

} // namespace Wayland

