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
#include "wlinputdevice.h"
#include "wlextendedsurface.h"

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

static const int buffer_pool_size = 3;

struct surface_buffer_destroy_listener
{
    struct wl_listener listener;
    class SurfaceBuffer *surfaceBuffer;
};


class SurfaceBuffer
{
    static int counter;
public:
    SurfaceBuffer()
        : m_buffer(0)
        , m_texture(0)
        , m_is_released_sent(false)
        , m_is_registered_for_buffer(false)
    {
    }

    ~SurfaceBuffer()
    {
        if (m_is_registered_for_buffer)
            destructBufferState();
    }

    void initialize(struct wl_buffer *buffer)
    {
        m_buffer = buffer;
        m_texture = 0;
        m_is_released_sent = false;
        m_is_registered_for_buffer = true;
        m_destroy_listener.surfaceBuffer = this;
        m_destroy_listener.listener.func = destroy_listener_callback;
        wl_list_insert(&buffer->resource.destroy_listener_list,&m_destroy_listener.listener.link);
    }

    void destructBufferState()
    {
        destroyTexture();
        if (m_buffer) {
            wl_list_remove(&m_destroy_listener.listener.link);
            sendRelease();
        }
        m_buffer = 0;
        m_is_registered_for_buffer = false;
    }


    inline int32_t width() const { return m_buffer->width; }
    inline int32_t height() const { return m_buffer->height; }

    inline bool bufferIsDestroyed() const { return m_is_registered_for_buffer &&!m_buffer; }
    inline bool isShmBuffer() const { return wl_buffer_is_shm(m_buffer); }

    inline bool isReleasedSent() const { return m_is_released_sent; }
    inline bool isRegisteredWithBuffer() const { return m_is_registered_for_buffer; }

    void sendRelease()
    {
        Q_ASSERT(m_buffer);
        wl_resource_post_event(&m_buffer->resource, WL_BUFFER_RELEASE);
        m_buffer = 0;
    }

    inline bool textureCreated() const
    {
        return m_texture;
    }

    inline GLuint texture()
    {
        if (m_buffer)
            return m_texture;
        return 0;
    }

    void setTexture(GLuint texId)
    {
        m_texture = texId;
    }

    inline void destroyTexture()
    {
#ifdef QT_COMPOSITOR_WAYLAND_GL
        if (m_texture) {
            glDeleteTextures(1,&m_texture);
            m_texture = 0;
        }
#endif
    }

    inline struct wl_buffer *handle() const { return m_buffer; }
private:
    struct wl_buffer *m_buffer;
    struct surface_buffer_destroy_listener m_destroy_listener;
    bool m_is_released_sent;
    bool m_is_registered_for_buffer;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint m_texture;
#else
    uint m_texture;
#endif

    static void destroy_listener_callback(struct wl_listener *listener,
             struct wl_resource *resource, uint32_t time)
    {
        Q_UNUSED(resource);
        Q_UNUSED(time);
        struct surface_buffer_destroy_listener *destroy_listener =
                reinterpret_cast<struct surface_buffer_destroy_listener *>(listener);
        SurfaceBuffer *d = destroy_listener->surfaceBuffer;
        d->destroyTexture();
        d->m_buffer = 0;
    }
};

int SurfaceBuffer::counter = 0;


const struct wl_surface_interface Surface::surface_interface = {
        Surface::surface_destroy,
        Surface::surface_attach,
        Surface::surface_damage,
        Surface::surface_frame
};

class SurfacePrivate
{
    Q_DECLARE_PUBLIC(Surface)
public:
    SurfacePrivate(Surface *surface, struct wl_client *client, Compositor *compositor)
        : q_ptr(surface)
        , client(client)
        , compositor(compositor)
        , directRenderBuffer(0)
        , processId(0)
        , textureBuffer(0)
        , surfaceBuffer(0)
        , extendedSurface(0)


    {
        wl_list_init(&frame_callback_list);
    }

    static void destroy_frame_callback(struct wl_resource *resource)
    {
        delete resource;
    }

    struct wl_client *client;
    Compositor *compositor;
    WaylandSurface *qtSurface;

    SurfaceBuffer  *surfaceBuffer;
    SurfaceBuffer *textureBuffer;
    SurfaceBuffer *directRenderBuffer;

    qint64 processId;
    QByteArray authenticationToken;
    QVariantMap windowProperties;

    QPoint lastMousePos;

    struct wl_list frame_callback_list;

    ExtendedSurface *extendedSurface;

    SurfaceBuffer bufferPool[buffer_pool_size];
private:
    Surface *q_ptr;
};


void Surface::surface_destroy(struct wl_client *client, struct wl_resource *surface_resource)
{
    Surface *surface = reinterpret_cast<Surface *>(surface_resource);
    wl_resource_destroy(surface_resource,Compositor::currentTimeMsecs());
}

void Surface::surface_attach(struct wl_client *client, struct wl_resource *surface,
                    struct wl_resource *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);
    reinterpret_cast<Surface *>(surface)->attach(reinterpret_cast<struct wl_buffer *>(buffer));
}

void Surface::surface_damage(struct wl_client *client, struct wl_resource *surface,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    reinterpret_cast<Surface *>(surface)->damage(QRect(x, y, width, height));
}
void Surface::surface_frame(struct wl_client *client,
                   struct wl_resource *resource,
                   uint32_t callback)
{
    Surface *surface = reinterpret_cast<Surface *>(resource);
    SurfacePrivate *d = surface->d_func();
    struct wl_resource *frame_callback = wl_client_add_object(client,&wl_callback_interface,0,callback,d);
    wl_list_insert(&d->frame_callback_list,&frame_callback->link);
}

Surface::Surface(struct wl_client *client, Compositor *compositor)
    : d_ptr(new SurfacePrivate(this,client,compositor))
{
    base()->resource.client = client;
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
    if (this == d->compositor->directRenderSurface()) {
        return WaylandSurface::Direct;
    } else if (d->surfaceBuffer && !d->surfaceBuffer->bufferIsDestroyed()) {
        if (d->surfaceBuffer && d->surfaceBuffer->isShmBuffer()) {
            return WaylandSurface::Shm;
        } else if (d->surfaceBuffer){
            return WaylandSurface::Texture;
        }
    }
    return WaylandSurface::Invalid;
}

bool Surface::isYInverted() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    Q_D(const Surface);

    if (d->compositor->graphicsHWIntegration() && !d->surfaceBuffer->bufferIsDestroyed()) {
        if (type() == WaylandSurface::Texture) {
                return d->compositor->graphicsHWIntegration()->isYInverted(d->surfaceBuffer->handle());
        } else if (type() == WaylandSurface::Direct) {
            return d->compositor->graphicsHWIntegration()->isYInverted(d->surfaceBuffer->handle());
        }
    }
#endif
    //shm surfaces are not flipped (in our "world")
    return false;
}

bool Surface::visible() const
{
    Q_D(const Surface);
    return d->surfaceBuffer && d->surfaceBuffer->handle();
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);
    if (!d->surfaceBuffer || !d->surfaceBuffer->handle())
            return;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (d->compositor->graphicsHWIntegration() && type() == WaylandSurface::Direct) {
        //should the texture be deleted here, or should we explicitly delete it
        //when going into direct mode...
        if (d->surfaceBuffer->textureCreated()) {
            d->surfaceBuffer->destroyTexture();
        }
        if (d->textureBuffer) { //previousBuffer means previous buffer turned into texture
            d->textureBuffer->destructBufferState();
            d->textureBuffer = 0;
        }
        if (d->compositor->graphicsHWIntegration()->postBuffer(d->surfaceBuffer->handle())) {
            d->directRenderBuffer->destructBufferState();
            d->directRenderBuffer = d->surfaceBuffer;
            d->surfaceBuffer = 0;
            emit d->qtSurface->damaged(rect);
            return;
        }
    }
#endif
    d->directRenderBuffer = 0;
    d->compositor->markSurfaceAsDirty(this);

    emit d->qtSurface->damaged(rect);
}

QImage Surface::image() const
{
    Q_D(const Surface);
    if (type() == WaylandSurface::Shm && d->surfaceBuffer && d->surfaceBuffer->handle()) {
        ShmBuffer *shmBuffer = static_cast<ShmBuffer *>(d->surfaceBuffer->handle()->user_data);
        return shmBuffer->image();
    }
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint Surface::textureId(QOpenGLContext *context) const
{
    Q_D(const Surface);

    if (d->compositor->graphicsHWIntegration() && type() == WaylandSurface::Texture
         && !d->surfaceBuffer->textureCreated()) {
        Surface *that = const_cast<Surface *>(this);

        if (d->textureBuffer) {
            d->textureBuffer->destructBufferState();
            that->d_func()->textureBuffer = 0;
        }
        if (d->directRenderBuffer) {
            d->directRenderBuffer->destructBufferState();
            that->d_func()->directRenderBuffer = 0;
        }
        GraphicsHardwareIntegration *hwIntegration = d->compositor->graphicsHWIntegration();
        that->d_func()->textureBuffer = d->surfaceBuffer;
        that->d_func()->textureBuffer->setTexture(hwIntegration->createTextureFromBuffer(d->textureBuffer->handle(), context));
    }
    return d->textureBuffer->texture();
}
#endif // QT_COMPOSITOR_WAYLAND_GL

void Surface::attach(struct wl_buffer *buffer)
{
    Q_D(Surface);
    SurfaceBuffer *newBuffer = 0;
    if (buffer) {
        for (int i = 0; i < buffer_pool_size; i++) {
            if (!d->bufferPool[i].isRegisteredWithBuffer()) {
                newBuffer = &d->bufferPool[i];
                newBuffer->initialize(buffer);
                break;
            }
        }
        Q_ASSERT(newBuffer);
    }

    bool emitMap = !d->surfaceBuffer && buffer;
    bool emitUnmap = d->surfaceBuffer && !buffer;

    if (d->surfaceBuffer && d->surfaceBuffer != d->directRenderBuffer) {
        if (d->textureBuffer == d->surfaceBuffer)
            d->textureBuffer = 0;
        d->surfaceBuffer->destructBufferState();
        d->surfaceBuffer = 0;
    }
    d->surfaceBuffer = newBuffer;
    if (emitMap) {
        d->qtSurface->mapped();
    } else if (emitUnmap) {
        d->qtSurface->unmapped();
    }
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
    return d->compositor->windowManagerIntegration()->managedClient(d->client)->authenticationToken();
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
    handle()->windowPropertyChanged(name,value);
    if (writeUpdateToClient && d->extendedSurface) {
        const char *property = qPrintable(name);
        d->extendedSurface->sendGenericProperty(property,value);
    }
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

void Surface::setExtendedSurface(ExtendedSurface *extendedSurface)
{
    Q_D(Surface);
    d->extendedSurface = extendedSurface;
}

ExtendedSurface *Surface::extendedSurface() const
{
    Q_D(const Surface);
    return d->extendedSurface;
}

void Surface::sendMousePressEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(Surface);
    sendMouseMoveEvent(x, y);
    if (d->client) {
        uint32_t time = d->compositor->currentTimeMsecs();
        struct wl_resource *pointer_focus_resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
        if (pointer_focus_resource) {
            wl_resource_post_event(d->compositor->defaultInputDevice()->base()->pointer_focus_resource,
                                   WL_INPUT_DEVICE_BUTTON, time, toWaylandButton(button), 1);
        }
    }
}

void Surface::sendMouseReleaseEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(Surface);
    sendMouseMoveEvent(x, y);
    if (d->client) {
        uint32_t time = d->compositor->currentTimeMsecs();
        struct wl_resource *pointer_focus_resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
        if (pointer_focus_resource) {
            wl_resource_post_event(pointer_focus_resource,
                                   WL_INPUT_DEVICE_BUTTON, time, toWaylandButton(button), 0);
        }
    }
}

void Surface::sendMouseMoveEvent(int x, int y)
{
    Q_D(Surface);
    if (d->client) {
        d->lastMousePos = QPoint(x, y);
        uint32_t time = d->compositor->currentTimeMsecs();
        d->compositor->setPointerFocus(this, QPoint(x, y));
        struct wl_resource *pointer_focus_resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
        if (pointer_focus_resource) {
            wl_resource_post_event(pointer_focus_resource,
                                   WL_INPUT_DEVICE_MOTION, time, x, y, x, y);
        }
    }
}

void Surface::sendKeyPressEvent(uint code)
{
    Q_D(Surface);
    if (d->compositor->defaultInputDevice()->base()->keyboard_focus_resource != NULL) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_resource_post_event(d->compositor->defaultInputDevice()->base()->keyboard_focus_resource,
                               WL_INPUT_DEVICE_KEY, time, code - 8, 1);
    }
}

void Surface::sendKeyReleaseEvent(uint code)
{
    Q_D(Surface);
    if (d->compositor->defaultInputDevice()->base()->keyboard_focus_resource != NULL) {
        uint32_t time = d->compositor->currentTimeMsecs();
        wl_resource_post_event(d->compositor->defaultInputDevice()->base()->keyboard_focus_resource,
                               WL_INPUT_DEVICE_KEY, time, code - 8, 0);
    }
}

void Surface::sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state)
{
    Q_D(Surface);
    uint32_t time = d->compositor->currentTimeMsecs();
    struct wl_resource *resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
    switch (state) {
    case Qt::TouchPointPressed:
        wl_resource_post_event(resource, WL_INPUT_DEVICE_TOUCH_DOWN, time, this, id, x, y);
        break;
    case Qt::TouchPointMoved:
        wl_resource_post_event(resource, WL_INPUT_DEVICE_TOUCH_MOTION, time, id, x, y);
        break;
    case Qt::TouchPointReleased:
        wl_resource_post_event(resource, WL_INPUT_DEVICE_TOUCH_UP, time, id);
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
    struct wl_resource *resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
    wl_resource_post_event(resource,
                         WL_INPUT_DEVICE_TOUCH_FRAME);
}

void Surface::sendTouchCancelEvent()
{
    Q_D(Surface);
    struct wl_resource *resource = d->compositor->defaultInputDevice()->base()->pointer_focus_resource;
    wl_resource_post_event(resource,
                         WL_INPUT_DEVICE_TOUCH_CANCEL);
}

void Surface::sendFrameCallback()
{
    Q_D(Surface);
    uint time = Compositor::currentTimeMsecs();
    struct wl_resource *frame_callback;
    wl_list_for_each(frame_callback, &d->frame_callback_list, link) {
        wl_resource_post_event(frame_callback,WL_CALLBACK_DONE,time);
        wl_resource_destroy(frame_callback,Compositor::currentTimeMsecs());
    }

    wl_list_init(&d->frame_callback_list);
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
    Q_D(Surface);
    if (d->extendedSurface) {
        d->extendedSurface->sendOnScreenVisibllity(visible);
    }
}

} // namespace Wayland

