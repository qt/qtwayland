/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlsurface_p.h"

#include "qwaylandsurface.h"
#ifdef QT_COMPOSITOR_QUICK
#include "qwaylandsurfaceitem.h"
#endif

#include "qwlcompositor_p.h"
#include "qwlinputdevice_p.h"
#include "qwlextendedsurface_p.h"
#include "qwlregion_p.h"
#include "qwlsubsurface_p.h"
#include "qwlsurfacebuffer_p.h"
#include "qwlshellsurface_p.h"

#include <QtCore/QDebug>
#include <QTouchEvent>

#include <wayland-server.h>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "hardware_integration/qwaylandgraphicshardwareintegration.h"
#include <qpa/qplatformopenglcontext.h>
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "waylandwindowmanagerintegration.h"
#endif

QT_BEGIN_NAMESPACE

namespace QtWayland {

void destroy_surface(struct wl_resource *resource)
{
    Surface *surface = resolve<Surface>(resource);
    surface->compositor()->surfaceDestroyed(surface);
    delete surface;
}

Surface::Surface(struct wl_client *client, uint32_t id, Compositor *compositor)
    : m_compositor(compositor)
    , m_waylandSurface(new QWaylandSurface(this))
    , m_backBuffer(0)
    , m_frontBuffer(0)
    , m_surfaceMapped(false)
    , m_extendedSurface(0)
    , m_subSurface(0)
    , m_shellSurface(0)
    , m_transientInactive(false)
    , m_isCursorSurface(false)
{
    wl_list_init(&m_frame_callback_list);
    addClientResource(client, &base()->resource, id, &wl_surface_interface,
            &Surface::surface_interface, destroy_surface);
    for (int i = 0; i < buffer_pool_size; i++) {
        m_bufferPool[i] = new SurfaceBuffer(this);
    }
}

Surface::~Surface()
{
    delete m_waylandSurface;
    delete m_extendedSurface;
    delete m_subSurface;
    delete m_shellSurface;

    for (int i = 0; i < buffer_pool_size; i++) {
        if (!m_bufferPool[i]->pageFlipperHasBuffer())
            delete m_bufferPool[i];
    }
}

QWaylandSurface::Type Surface::type() const
{
    SurfaceBuffer *surfaceBuffer = currentSurfaceBuffer();
    if (surfaceBuffer && surfaceBuffer->waylandBufferHandle()) {
        if (surfaceBuffer->isShmBuffer()) {
            return QWaylandSurface::Shm;
        } else {
            return QWaylandSurface::Texture;
        }
    }
    return QWaylandSurface::Invalid;
}

bool Surface::isYInverted() const
{
    bool ret = false;
    static bool negateReturn = qgetenv("QT_COMPOSITOR_NEGATE_INVERTED_Y").toInt();
    QWaylandGraphicsHardwareIntegration *graphicsHWIntegration = m_compositor->graphicsHWIntegration();

#ifdef QT_COMPOSITOR_WAYLAND_GL
    SurfaceBuffer *surfacebuffer = currentSurfaceBuffer();
    if (!surfacebuffer) {
        ret = false;
    } else if (graphicsHWIntegration && surfacebuffer->waylandBufferHandle() && type() != QWaylandSurface::Shm) {
        ret = graphicsHWIntegration->isYInverted(surfacebuffer->waylandBufferHandle());
    } else
#endif
        ret = true;

    return ret != negateReturn;
}

bool Surface::visible() const
{

    SurfaceBuffer *surfacebuffer = currentSurfaceBuffer();
    return surfacebuffer->waylandBufferHandle();
}

QPointF Surface::pos() const
{
    return m_shellSurface ? m_shellSurface->adjustedPosToTransientParent() : m_position;
}

QPointF Surface::nonAdjustedPos() const
{
    return m_position;
}

void Surface::setPos(const QPointF &pos)
{
    bool emitChange = pos != m_position;
    m_position = pos;
    if (emitChange)
        m_waylandSurface->posChanged();
}

QSize Surface::size() const
{
    return m_size;
}

void Surface::setSize(const QSize &size)
{
    if (size != m_size) {
        m_opaqueRegion = QRegion();
        m_inputRegion = QRegion(QRect(QPoint(), size));
        m_size = size;
        if (m_shellSurface) {
            m_shellSurface->adjustPosInResize();
        }
        m_waylandSurface->sizeChanged();
    }
}

QRegion Surface::inputRegion() const
{
    return m_inputRegion;
}

QRegion Surface::opaqueRegion() const
{
    return m_opaqueRegion;
}

QImage Surface::image() const
{
    SurfaceBuffer *surfacebuffer = currentSurfaceBuffer();
    if (surfacebuffer && !surfacebuffer->isDestroyed() && type() == QWaylandSurface::Shm) {
        struct wl_buffer *buffer = surfacebuffer->waylandBufferHandle();
        int stride = wl_shm_buffer_get_stride(buffer);
        uint format = wl_shm_buffer_get_format(buffer);
        (void) format;
        void *data = wl_shm_buffer_get_data(buffer);
        const uchar *char_data = static_cast<const uchar *>(data);
        QImage img(char_data, buffer->width, buffer->height, stride, QImage::Format_ARGB32_Premultiplied);
        return img;
    }
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint Surface::textureId(QOpenGLContext *context) const
{
    const SurfaceBuffer *surfacebuffer = currentSurfaceBuffer();

    if (m_compositor->graphicsHWIntegration() && type() == QWaylandSurface::Texture
         && !surfacebuffer->textureCreated()) {
        QWaylandGraphicsHardwareIntegration *hwIntegration = m_compositor->graphicsHWIntegration();
        const_cast<SurfaceBuffer *>(surfacebuffer)->createTexture(hwIntegration,context);
    }
    return surfacebuffer->texture();
}
#endif // QT_COMPOSITOR_WAYLAND_GL

void Surface::sendFrameCallback()
{
    SurfaceBuffer *surfacebuffer = currentSurfaceBuffer();
    surfacebuffer->setDisplayed();
    if (m_backBuffer) {
        if (m_frontBuffer)
            m_frontBuffer->disown();
        m_frontBuffer = m_backBuffer;
    }

    bool updateNeeded = advanceBufferQueue();

    uint time = Compositor::currentTimeMsecs();
    struct wl_resource *frame_callback, *next;
    wl_list_for_each_safe(frame_callback, next, &m_frame_callback_list, link) {
        wl_callback_send_done(frame_callback, time);
        wl_resource_destroy(frame_callback);
    }
    wl_list_init(&m_frame_callback_list);

    if (updateNeeded)
        doUpdate();
}

void Surface::frameFinished()
{
    m_compositor->frameFinished(this);
}

QWaylandSurface * Surface::waylandSurface() const
{
    return m_waylandSurface;
}

QPoint Surface::lastMousePos() const
{
    return m_lastLocalMousePos;
}

void Surface::setExtendedSurface(ExtendedSurface *extendedSurface)
{
    m_extendedSurface = extendedSurface;
    if (m_extendedSurface)
        emit m_waylandSurface->extendedSurfaceReady();
}

ExtendedSurface *Surface::extendedSurface() const
{
    return m_extendedSurface;
}

void Surface::setSubSurface(SubSurface *subSurface)
{
    m_subSurface = subSurface;
}

SubSurface *Surface::subSurface() const
{
    return m_subSurface;
}

void Surface::setShellSurface(ShellSurface *shellSurface)
{
    m_shellSurface = shellSurface;
}

ShellSurface *Surface::shellSurface() const
{
    return m_shellSurface;
}

Compositor *Surface::compositor() const
{
    return m_compositor;
}

bool Surface::advanceBufferQueue()
{
    //has current buffer been displayed,
    //do we have another buffer in the queue
    //and does it have a valid damage rect

    if (m_bufferQueue.size()) {
        int width = 0;
        int height = 0;
        if (m_backBuffer && m_backBuffer->waylandBufferHandle()) {
            width = m_backBuffer->width();
            height = m_backBuffer->height();
        }

        m_backBuffer = m_bufferQueue.takeFirst();
        while (m_backBuffer && m_backBuffer->isDestroyed()) {
            m_backBuffer->disown();
            m_backBuffer = m_bufferQueue.size() ? m_bufferQueue.takeFirst() : 0;
        }

        if (!m_backBuffer)
            return false; //we have no new backbuffer;

        if (m_backBuffer->waylandBufferHandle()) {
            width = m_backBuffer->width();
            height = m_backBuffer->height();
        }
        setSize(QSize(width,height));


        if (m_backBuffer &&  (!m_subSurface || !m_subSurface->parent()) && !m_surfaceMapped) {
            m_surfaceMapped = true;
            emit m_waylandSurface->mapped();
        } else if (m_backBuffer && !m_backBuffer->waylandBufferHandle() && m_surfaceMapped) {
            m_surfaceMapped = false;
            emit m_waylandSurface->unmapped();
        }

    } else {
        m_backBuffer = 0;
        return false;
    }

    return true;
}

void Surface::doUpdate() {
    if (postBuffer()) {
#ifdef QT_COMPOSITOR_QUICK
        QWaylandSurfaceItem *surfaceItem = waylandSurface()->surfaceItem();
        if (surfaceItem)
            surfaceItem->setDamagedFlag(true); // avoid flicker when we switch back to composited mode
#endif
        sendFrameCallback();
    } else {
        SurfaceBuffer *surfaceBuffer = currentSurfaceBuffer();
        if (surfaceBuffer) {
            if (surfaceBuffer->damageRect().isValid()) {
                m_compositor->markSurfaceAsDirty(this);
                emit m_waylandSurface->damaged(surfaceBuffer->damageRect());
            }
        }
    }
}

SurfaceBuffer *Surface::createSurfaceBuffer(struct wl_buffer *buffer)
{
    SurfaceBuffer *newBuffer = 0;
    for (int i = 0; i < Surface::buffer_pool_size; i++) {
        if (!m_bufferPool[i]->isRegisteredWithBuffer()) {
            newBuffer = m_bufferPool[i];
            newBuffer->initialize(buffer);
            break;
        }
    }

    Q_ASSERT(newBuffer);
    return newBuffer;
}

bool Surface::postBuffer() {
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_waylandSurface->handle() == m_compositor->directRenderSurface()) {
        SurfaceBuffer *surfaceBuffer = currentSurfaceBuffer();
        if (surfaceBuffer && surfaceBuffer->waylandBufferHandle()) {
            if (m_compositor->pageFlipper()) {
                if (m_compositor->pageFlipper()->displayBuffer(surfaceBuffer)) {
                    surfaceBuffer->setPageFlipperHasBuffer(true);
                    m_compositor->setDirectRenderingActive(true);
                    return true;
                } else {
                    qDebug() << "could not post buffer";
                }
            }
        }
    }
#endif
    return false;
}

void Surface::commit()
{
    if (!m_bufferQueue.isEmpty() && !m_backBuffer)
        advanceBufferQueue();

    doUpdate();
}

void Surface::attach(struct wl_buffer *buffer)
{
    SurfaceBuffer *last = m_bufferQueue.size()?m_bufferQueue.last():0;
    if (last) {
        if (last->waylandBufferHandle() == buffer)
            return;
        if (!last->damageRect().isValid() || isCursorSurface() ){
            last->disown();
            m_bufferQueue.takeLast();
        }
    }

    m_bufferQueue <<  createSurfaceBuffer(buffer);

    if (!buffer) {
        InputDevice *inputDevice = m_compositor->defaultInputDevice();
        if (inputDevice->keyboardFocus() == this)
            inputDevice->setKeyboardFocus(0);
        if (inputDevice->mouseFocus() == this)
            inputDevice->setMouseFocus(0, QPointF(), QPointF());
    }
}

void Surface::damage(const QRect &rect)
{
    SurfaceBuffer *surfaceBuffer = m_bufferQueue.isEmpty() ? currentSurfaceBuffer() : m_bufferQueue.last();
    if (surfaceBuffer)
        surfaceBuffer->setDamage(rect);
    else
        qWarning() << "Surface::damage() null buffer";
}

const struct wl_surface_interface Surface::surface_interface = {
        Surface::surface_destroy,
        Surface::surface_attach,
        Surface::surface_damage,
        Surface::surface_frame,
        Surface::surface_set_opaque_region,
        Surface::surface_set_input_region,
        Surface::surface_commit
};

void Surface::surface_destroy(struct wl_client *, struct wl_resource *surface_resource)
{
    wl_resource_destroy(surface_resource);
}

void Surface::surface_attach(struct wl_client *client, struct wl_resource *surface,
                    struct wl_resource *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);
    resolve<Surface>(surface)->attach(buffer ? reinterpret_cast<wl_buffer *>(buffer->data) : 0);
}

void Surface::surface_damage(struct wl_client *client, struct wl_resource *surface,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    resolve<Surface>(surface)->damage(QRect(x, y, width, height));
}

void Surface::surface_frame(struct wl_client *client,
                   struct wl_resource *resource,
                   uint32_t callback)
{
    Surface *surface = resolve<Surface>(resource);
    struct wl_resource *frame_callback = wl_client_add_object(client,&wl_callback_interface,0,callback,surface);
    wl_list_insert(&surface->m_frame_callback_list,&frame_callback->link);
}

void Surface::surface_set_opaque_region(struct wl_client *client, struct wl_resource *surfaceResource,
                                        struct wl_resource *region)
{
    Q_UNUSED(client);
    Surface *surface = resolve<Surface>(surfaceResource);
    surface->m_opaqueRegion = region ? resolve<Region>(region)->region() : QRegion();
}

void Surface::surface_set_input_region(struct wl_client *client, struct wl_resource *surfaceResource,
                                       struct wl_resource *region)
{
    Q_UNUSED(client);
    Surface *surface = resolve<Surface>(surfaceResource);
    surface->m_inputRegion = region ? resolve<Region>(region)->region() : QRegion(QRect(QPoint(), surface->size()));
}

void Surface::surface_commit(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    resolve<Surface>(resource)->commit();
}

void Surface::setClassName(const QString &className)
{
    if (m_className != className) {
        m_className = className;
        emit waylandSurface()->classNameChanged();
    }
}

void Surface::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit waylandSurface()->titleChanged();
    }
}

} // namespace Wayland

QT_END_NAMESPACE
