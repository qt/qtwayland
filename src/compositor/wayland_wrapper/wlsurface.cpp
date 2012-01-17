/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wlsurface.h"

#include "waylandsurface.h"

#include "wlcompositor.h"
#include "wlshmbuffer.h"
#include "wlinputdevice.h"
#include "wlextendedsurface.h"
#include "wlsubsurface.h"

#include <QtCore/QDebug>
#include <QTouchEvent>

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
public:
    SurfaceBuffer()
        : m_buffer(0)
        , m_is_released_sent(false)
        , m_is_registered_for_buffer(false)
        , m_is_posted(false)
        , m_is_frame_finished(false)
        , m_texture(0)
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
        m_is_posted = false;
        m_is_frame_finished = false;
        m_destroy_listener.surfaceBuffer = this;
        m_destroy_listener.listener.func = destroy_listener_callback;
        wl_list_insert(&buffer->resource.destroy_listener_list,&m_destroy_listener.listener.link);
        m_damageRect = QRect();
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
        m_is_posted = 0;
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
        m_is_released_sent = true;
    }

    void setPosted() {
        m_is_posted = true;
         if (m_buffer) {
            wl_list_remove(&m_destroy_listener.listener.link);
         }
         m_buffer = 0;
    }

    void setFinished() { m_is_frame_finished = true; }

    inline bool isPosted() const { return m_is_posted; }
    inline bool isDisplayed() const { return m_texture || m_is_posted || wl_buffer_is_shm(m_buffer); }
    inline bool isFinished() const { return m_is_frame_finished; }

    inline QRect damageRect() const { return m_damageRect; }

    inline void setDamage(const QRect &rect) { m_damageRect = rect; }

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

    inline void setTexture(GLuint texId)
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
    QRect m_damageRect;
    bool m_is_released_sent;
    bool m_is_registered_for_buffer;
    bool m_is_posted;
    bool m_is_frame_finished;
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
    SurfacePrivate(Surface *surface, Compositor *compositor)
        : compositor(compositor)
        , qtSurface(new WaylandSurface(surface))
        , surfaceBuffer(0)
        , textureBuffer(0)
        , surfaceMapped(false)
        , processId(0)
        , extendedSurface(0)
        , subSurface(0)
        , shellSurface(0)
        , q_ptr(surface)

    {
        wl_list_init(&frame_callback_list);
    }

    static void destroy_frame_callback(struct wl_resource *resource)
    {
        delete resource;
    }

    void doUpdate(const QRect &rect) {
        if (postBuffer()) {
            surfaceBuffer->setPosted();  // disown buffer....
            if (textureBuffer) {
                textureBuffer->destructBufferState();
                textureBuffer = 0;
            }
            if (!bufferQueue.isEmpty()) {
                qDebug() << "++++++++++++++++++++++++++++++++++++++++ recursive damage :-)";
                newCurrentBuffer();
                doUpdate(rect);
            }
        } else {
            compositor->markSurfaceAsDirty(q_ptr);
            emit qtSurface->damaged(rect);
        }
    }

    void newCurrentBuffer() {
        //TODO release SHM buffer....
        if (surfaceBuffer && surfaceBuffer->isPosted()) {
            surfaceBuffer->destructBufferState();
        } else if (surfaceBuffer && !surfaceBuffer->isDisplayed()) {
            qDebug() << "### not skipping undisplayed buffer";
            return;
        }

        surfaceBuffer = bufferQueue.takeFirst();

        int width = 0;
        int height = 0;
        if (surfaceBuffer) {
            width = surfaceBuffer->width();
            height = surfaceBuffer->height();
        }
        q_ptr->setSize(QSize(width,height));

        if (surfaceBuffer &&  (!subSurface || !subSurface->parent()) && !surfaceMapped) {
            emit qtSurface->mapped();
            surfaceMapped = true;
        } else if (!surfaceBuffer && surfaceMapped) {
            emit qtSurface->unmapped();
            surfaceMapped = false;
        }
    }

    SurfaceBuffer *createSurfaceBuffer(struct wl_buffer *buffer)
     {
         SurfaceBuffer *newBuffer = 0;
         for (int i = 0; i < buffer_pool_size; i++) {
            if (!bufferPool[i].isRegisteredWithBuffer()) {
                newBuffer = &bufferPool[i];
                newBuffer->initialize(buffer);
                break;
            }
        }
         if (!newBuffer) {
             qDebug() << "####################### create failed ######################";
         }
         return newBuffer;
     }

    void frameFinished() {
        if (surfaceBuffer)
            surfaceBuffer->setFinished();

        if (!bufferQueue.isEmpty()) {
            newCurrentBuffer();
            if (surfaceBuffer)
                doUpdate(surfaceBuffer->damageRect());
        }
    }

    bool postBuffer() {
#ifdef QT_COMPOSITOR_WAYLAND_GL
        if (compositor->graphicsHWIntegration() && qtSurface->handle() == compositor->directRenderSurface()) {
//            qDebug() << "posting...." << bufferQueue;
            if (surfaceBuffer && surfaceBuffer->handle() && compositor->graphicsHWIntegration()->postBuffer(surfaceBuffer->handle())) {
                return true;
            } else {
                qDebug() << "could not post buffer";
            }
        }
#endif
        return false;
    }

    Compositor *compositor;
    WaylandSurface *qtSurface;

    SurfaceBuffer  *surfaceBuffer;
    SurfaceBuffer *textureBuffer;
    QList<SurfaceBuffer*> bufferQueue;
    bool surfaceMapped;

    qint64 processId;
    QByteArray authenticationToken;
    QVariantMap windowProperties;

    QPoint lastLocalMousePos;
    QPoint lastGlobalMousePos;

    struct wl_list frame_callback_list;

    ExtendedSurface *extendedSurface;
    SubSurface *subSurface;
    ShellSurface *shellSurface;

    SurfaceBuffer bufferPool[buffer_pool_size];

    QPointF position;
    QSize size;

private:
    Surface *q_ptr;
};

void destroy_surface(struct wl_resource *resource)
{
    Surface *surface = wayland_cast<Surface *>((wl_surface *)resource);
    delete surface;
}

void Surface::surface_destroy(struct wl_client *, struct wl_resource *surface_resource)
{
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

Surface::Surface(struct wl_client *client, uint32_t id, Compositor *compositor)
    : d_ptr(new SurfacePrivate(this,compositor))
{
    addClientResource(client, &base()->resource, id, &wl_surface_interface,
            &Surface::surface_interface, destroy_surface);
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
    if (d->surfaceBuffer && d->surfaceBuffer->handle()) {
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

    if (!d->surfaceBuffer)
        return false;
    if (d->compositor->graphicsHWIntegration() && d->surfaceBuffer->handle() && type() != WaylandSurface::Shm) {
        return d->compositor->graphicsHWIntegration()->isYInverted(d->surfaceBuffer->handle());
    }
#endif
    return true;
}

bool Surface::visible() const
{
    Q_D(const Surface);
    return d->surfaceBuffer && d->surfaceBuffer->handle();
}

void Surface::damage(const QRect &rect)
{
    Q_D(Surface);

    if (!d->bufferQueue.isEmpty() && (!d->surfaceBuffer || d->surfaceBuffer->isFinished() || !d->surfaceBuffer->handle())) {
            // Handle the "slow" case where we've finished the previous frame before the next damage comes.
            d->newCurrentBuffer();
            d->doUpdate(rect);
    } else if (d->bufferQueue.isEmpty()) {
        // we've receicved a second damage for the same buffer
        d->doUpdate(rect);
    } else {
        // we're still composing the previous buffer, so just store the damage rect for later
        SurfaceBuffer *b = d->bufferQueue.last();
        if (b)
            b->setDamage(rect);
        else
            qWarning() << "Surface::damage() null buffer";
    }
}

QImage Surface::image() const
{
    Q_D(const Surface);
    if (type() == WaylandSurface::Shm && d->surfaceBuffer && d->surfaceBuffer->handle()) {
        ShmBuffer *shmBuffer = static_cast<ShmBuffer *>(d->surfaceBuffer->handle()->user_data);
        //TODO SHM: d->surfaceBuffer->bufferHandled = true;
        return shmBuffer->image();
    }
    return QImage();
}

QPointF Surface::pos() const
{
    Q_D(const Surface);
    return d->position;
}

void Surface::setPos(const QPointF &pos)
{
    Q_D(Surface);
    bool emitChange = pos != d->position;
    d->position = pos;
    if (emitChange)
        d->qtSurface->posChanged();
}

QSize Surface::size() const
{
    Q_D(const Surface);
    return d->size;
}

void Surface::setSize(const QSize &size)
{
    Q_D(Surface);
    bool emitChange = size != d->size;
    d->size = size;
    if (emitChange)
        d->qtSurface->sizeChanged();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint Surface::textureId(QOpenGLContext *context) const
{
    Q_D(const Surface);

    if (!d->surfaceBuffer) {
        return 0;
    }
    if (d->compositor->graphicsHWIntegration() && type() == WaylandSurface::Texture
         && !d->surfaceBuffer->textureCreated()) {
        Surface *that = const_cast<Surface *>(this);

        if (d->textureBuffer) {
            d->textureBuffer->destructBufferState();
            that->d_func()->textureBuffer = 0;
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
        newBuffer = d->createSurfaceBuffer(buffer);
        Q_ASSERT(newBuffer);
    }
#if 0 //GAMING_TRIPLE_BUFFERING
    if (d->surfaceBuffer && !d->surfaceBuffer->textureCreated()) {
        //qDebug() << "releasing undisplayed buffer";
        d->surfaceBuffer->destructBufferState();
        d->surfaceBuffer = 0;
    }
#endif
    SurfaceBuffer *last = d->bufferQueue.size()?d->bufferQueue.last():0;
    if (last && !last->damageRect().isValid()) {
        last->destructBufferState();
        d->bufferQueue.takeLast();
    }
    d->bufferQueue << newBuffer;
}

WaylandSurface * Surface::handle() const
{
    Q_D(const Surface);
    return d->qtSurface;
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
    WaylandManagedClient *mcl = d->compositor->windowManagerIntegration()->managedClient(base()->resource.client);
    return mcl ? mcl->authenticationToken() : QByteArray();
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

QPoint Surface::lastMousePos() const
{
    Q_D(const Surface);
    return d->lastLocalMousePos;
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

void Surface::setSubSurface(SubSurface *subSurface)
{
    Q_D(Surface);
    d->subSurface = subSurface;
}

SubSurface *Surface::subSurface() const
{
    Q_D(const Surface);
    return d->subSurface;
}

void Surface::setShellSurface(ShellSurface *shellSurface)
{
    Q_D(Surface);
    d->shellSurface = shellSurface;
}

ShellSurface *Surface::shellSurface() const
{
    Q_D(const Surface);
    return d->shellSurface;
}

Compositor *Surface::compositor() const
{
    Q_D(const Surface);
    return d->compositor;
}

void Surface::sendFrameCallback()
{
    Q_D(Surface);

    d->frameFinished();

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

void Surface::sendOnScreenVisibilityChange(bool visible)
{
    Q_D(Surface);
    if (d->extendedSurface) {
        d->extendedSurface->sendOnScreenVisibllity(visible);
    }
}

} // namespace Wayland

