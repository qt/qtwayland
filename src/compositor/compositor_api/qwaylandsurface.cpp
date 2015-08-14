/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"

#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"
#include "wayland_wrapper/qwlregion_p.h"

#include "extensions/qwlextendedsurface_p.h"
#include "extensions/qwlsubsurface_p.h"

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandView>
#include <QtCompositor/QWaylandBufferRef>

#include <QtCompositor/private/qwaylandcompositor_p.h>
#include <QtCompositor/private/qwaylandview_p.h>
#include <QtCompositor/private/qwaylandinput_p.h>

#include <QtCore/private/qobject_p.h>

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {
class FrameCallback {
public:
    FrameCallback(QWaylandSurface *surf, wl_resource *res)
        : surface(surf)
        , resource(res)
        , canSend(false)
    {
#if WAYLAND_VERSION_MAJOR < 1 || (WAYLAND_VERSION_MAJOR == 1 && WAYLAND_VERSION_MINOR <= 2)
        res->data = this;
        res->destroy = destroyCallback;
#else
        wl_resource_set_implementation(res, 0, this, destroyCallback);
#endif
    }
    ~FrameCallback()
    {
    }
    void destroy()
    {
        if (resource)
            wl_resource_destroy(resource);
        else
            delete this;
    }
    void send(uint time)
    {
        wl_callback_send_done(resource, time);
        wl_resource_destroy(resource);
    }
    static void destroyCallback(wl_resource *res)
    {
#if WAYLAND_VERSION_MAJOR < 1 || (WAYLAND_VERSION_MAJOR == 1 && WAYLAND_VERSION_MINOR <= 2)
        FrameCallback *_this = static_cast<FrameCallback *>(res->data);
#else
        FrameCallback *_this = static_cast<FrameCallback *>(wl_resource_get_user_data(res));
#endif
        QWaylandSurfacePrivate::get(_this->surface)->removeFrameCallback(_this);
        delete _this;
    }
    QWaylandSurface *surface;
    wl_resource *resource;
    bool canSend;
};
}
static QRegion infiniteRegion() {
    return QRegion(QRect(QPoint(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()),
                         QPoint(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())));
}

QWaylandSurfacePrivate::QWaylandSurfacePrivate(QWaylandClient *client, quint32 id, int version, QWaylandCompositor *compositor)
    : QtWaylandServer::wl_surface(client->client(), id, version)
    , m_compositor(compositor)
    , refCount(1)
    , client(client)
    , m_buffer(0)
    , m_inputPanelSurface(0)
    , m_inputRegion(infiniteRegion())
    , m_isCursorSurface(false)
    , m_destroyed(false)
    , m_contentOrientation(Qt::PrimaryOrientation)
{
    m_pending.buffer = 0;
    m_pending.newlyAttached = false;
    m_pending.inputRegion = infiniteRegion();
}

QWaylandSurfacePrivate::~QWaylandSurfacePrivate()
{
    for (int i = 0; i < views.size(); i++) {
        QWaylandViewPrivate::get(views.at(i))->markSurfaceAsDestroyed(q_func());
    }
    views.clear();

    m_bufferRef = QWaylandBufferRef();

    for (int i = 0; i < m_bufferPool.size(); i++)
        m_bufferPool[i]->setDestroyIfUnused(true);

    foreach (QtWayland::FrameCallback *c, m_pendingFrameCallbacks)
        c->destroy();
    foreach (QtWayland::FrameCallback *c, m_frameCallbacks)
        c->destroy();
}

void QWaylandSurfacePrivate::setSize(const QSize &size)
{
    Q_Q(QWaylandSurface);
    if (size != m_size) {
        m_opaqueRegion = QRegion();
        m_size = size;
        q->sizeChanged();
    }
}

void QWaylandSurfacePrivate::sendFrameCallback()
{
    uint time = m_compositor->currentTimeMsecs();
    foreach (QtWayland::FrameCallback *callback, m_frameCallbacks) {
        if (callback->canSend) {
            callback->send(time);
            m_frameCallbacks.removeOne(callback);
        }
    }
}

void QWaylandSurfacePrivate::removeFrameCallback(QtWayland::FrameCallback *callback)
{
    m_pendingFrameCallbacks.removeOne(callback);
    m_frameCallbacks.removeOne(callback);
}

void QWaylandSurfacePrivate::frameStarted()
{
    foreach (QtWayland::FrameCallback *c, m_frameCallbacks)
        c->canSend = true;
}

void QWaylandSurfacePrivate::notifyViewsAboutDestruction()
{
    Q_Q(QWaylandSurface);
    foreach (QWaylandView *view, views) {
        QWaylandViewPrivate::get(view)->markSurfaceAsDestroyed(q);
    }
}

void QWaylandSurfacePrivate::surface_destroy_resource(Resource *)
{
    Q_Q(QWaylandSurface);
    notifyViewsAboutDestruction();

    m_destroyed = true;
    q->destroy();
    emit q->surfaceDestroyed();
}

void QWaylandSurfacePrivate::surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandSurfacePrivate::surface_attach(Resource *, struct wl_resource *buffer, int x, int y)
{
    if (m_pending.buffer)
        m_pending.buffer->disown();
    m_pending.buffer = createSurfaceBuffer(buffer);
    m_pending.offset = QPoint(x, y);
    m_pending.newlyAttached = true;
}

void QWaylandSurfacePrivate::surface_damage(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    m_pending.damage = m_pending.damage.united(QRect(x, y, width, height));
}

void QWaylandSurfacePrivate::surface_frame(Resource *resource, uint32_t callback)
{
    Q_Q(QWaylandSurface);
    struct wl_resource *frame_callback = wl_resource_create(resource->client(), &wl_callback_interface, wl_callback_interface.version, callback);
    m_pendingFrameCallbacks << new QtWayland::FrameCallback(q, frame_callback);
}

void QWaylandSurfacePrivate::surface_set_opaque_region(Resource *, struct wl_resource *region)
{
    m_opaqueRegion = region ? QtWayland::Region::fromResource(region)->region() : QRegion();
}

void QWaylandSurfacePrivate::surface_set_input_region(Resource *, struct wl_resource *region)
{
    if (region) {
        m_pending.inputRegion = QtWayland::Region::fromResource(region)->region();
    } else {
        m_pending.inputRegion = infiniteRegion();
    }
}

void QWaylandSurfacePrivate::surface_commit(Resource *)
{
    Q_Q(QWaylandSurface);

    if (m_pending.buffer || m_pending.newlyAttached) {
        setBackBuffer(m_pending.buffer, m_pending.damage);
    }

    m_pending.buffer = 0;
    m_pending.offset = QPoint();
    m_pending.newlyAttached = false;
    m_pending.damage = QRegion();

    if (m_buffer)
        m_buffer->setCommitted();

    m_frameCallbacks << m_pendingFrameCallbacks;
    m_pendingFrameCallbacks.clear();

    m_inputRegion = m_pending.inputRegion.intersected(QRect(QPoint(), m_size));

    emit q->redraw();
}

void QWaylandSurfacePrivate::surface_set_buffer_transform(Resource *resource, int32_t orientation)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandSurface);
    QScreen *screen = QGuiApplication::primaryScreen();
    bool isPortrait = screen->primaryOrientation() == Qt::PortraitOrientation;
    Qt::ScreenOrientation oldOrientation = m_contentOrientation;
    switch (orientation) {
        case WL_OUTPUT_TRANSFORM_90:
            m_contentOrientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_180:
            m_contentOrientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_270:
            m_contentOrientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        default:
            m_contentOrientation = Qt::PrimaryOrientation;
    }
    if (m_contentOrientation != oldOrientation)
        emit q->contentOrientationChanged();
}

void QWaylandSurfacePrivate::setBackBuffer(QtWayland::SurfaceBuffer *buffer, const QRegion &damage)
{
    Q_Q(QWaylandSurface);
    QtWayland::SurfaceBuffer *oldBuffer = m_buffer;
    m_buffer = buffer;
    m_bufferRef = QWaylandBufferRef(m_buffer);

    if (m_buffer) {
        bool valid = m_buffer->waylandBufferHandle() != 0;
        if (valid)
            setSize(m_buffer->size());

        m_damage = damage.intersected(QRect(QPoint(), m_size));
    } else  {
        setSize(QSize());
        m_damage = QRect();
    }

    for (int i = 0; i < views.size(); i++) {
        views.at(i)->attach(m_bufferRef, m_damage);
    }

    emit q->damaged(m_damage);
    if (QtWayland::SurfaceBuffer::hasContent(oldBuffer) != QtWayland::SurfaceBuffer::hasContent(m_buffer))
        emit q->mappedChanged();
    if (!m_pending.offset.isNull())
        emit q->offsetForNextFrame(m_pending.offset);
}

QtWayland::SurfaceBuffer *QWaylandSurfacePrivate::createSurfaceBuffer(struct ::wl_resource *buffer)
{
    Q_Q(QWaylandSurface);
    QtWayland::SurfaceBuffer *newBuffer = 0;
    for (int i = 0; i < m_bufferPool.size(); i++) {
        if (!m_bufferPool[i]->isRegisteredWithBuffer()) {
            newBuffer = m_bufferPool[i];
            newBuffer->initialize(buffer);
            break;
        }
    }

    if (!newBuffer) {
        newBuffer = new QtWayland::SurfaceBuffer(q);
        newBuffer->initialize(buffer);
        m_bufferPool.append(newBuffer);
        if (m_bufferPool.size() > 3)
            qWarning() << "Increased buffer pool size to" << m_bufferPool.size() << "for surface" << q;
    }

    return newBuffer;
}

QWaylandSurface::QWaylandSurface(QWaylandClient *client, quint32 id, int version, QWaylandCompositor *compositor)
    : QObject(*new QWaylandSurfacePrivate(client, id, version, compositor))
{
}

QWaylandSurface::QWaylandSurface(QWaylandSurfacePrivate *dptr)
    : QObject(*dptr)
{
}

QWaylandSurface::~QWaylandSurface()
{
    Q_D(QWaylandSurface);
    QWaylandCompositorPrivate::get(d->m_compositor)->unregisterSurface(this);
    d->notifyViewsAboutDestruction();
}

QWaylandClient *QWaylandSurface::client() const
{
    Q_D(const QWaylandSurface);
    if (isDestroyed() || !compositor()->clients().contains(d->client))
        return Q_NULLPTR;

    return d->client;
}

bool QWaylandSurface::isMapped() const
{
    Q_D(const QWaylandSurface);
    return d->mapped();
}

QSize QWaylandSurface::size() const
{
    Q_D(const QWaylandSurface);
    return d->size();
}

Qt::ScreenOrientation QWaylandSurface::contentOrientation() const
{
    Q_D(const QWaylandSurface);
    return d->contentOrientation();
}

QWaylandSurface::Origin QWaylandSurface::origin() const
{
    Q_D(const QWaylandSurface);
    return d->origin();
}

QWaylandCompositor *QWaylandSurface::compositor() const
{
    Q_D(const QWaylandSurface);
    return d->compositor();
}

void QWaylandSurface::sendFrameCallbacks()
{
    Q_D(QWaylandSurface);
    d->sendFrameCallback();
}

bool QWaylandSurface::hasInputPanelSurface() const
{
    Q_D(const QWaylandSurface);

    return d->inputPanelSurface() != 0;
}

bool QWaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const QWaylandSurface);
    return d->inputRegion().contains(p);
}

void QWaylandSurface::destroy()
{
    deref();
}

bool QWaylandSurface::isDestroyed() const
{
    Q_D(const QWaylandSurface);
    return d->isDestroyed();
}

void QWaylandSurface::markAsCursorSurface(bool cursorSurface)
{
    Q_D(QWaylandSurface);
    d->setCursorSurface(cursorSurface);
}

bool QWaylandSurface::isCursorSurface() const
{
    Q_D(const QWaylandSurface);
    return d->isCursorSurface();
}

/*!
    Updates the surface with the compositor's retained clipboard selection. While this
    is done automatically when the surface receives keyboard focus, this function is
    useful for updating clients which do not have keyboard focus.
*/
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    QWaylandInputDevice *inputDevice = d->compositor()->defaultInputDevice();
    if (inputDevice) {
        const QtWayland::DataDevice *dataDevice = QWaylandInputDevicePrivate::get(inputDevice)->dataDevice();
        if (dataDevice) {
            QWaylandCompositorPrivate::get(d->compositor())->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}

void QWaylandSurface::ref()
{
    Q_D(QWaylandSurface);
    ++d->refCount;
}

void QWaylandSurface::deref()
{
    Q_D(QWaylandSurface);
    if (--d->refCount == 0)
        QWaylandCompositorPrivate::get(compositor())->destroySurface(this);
}

QWaylandView *QWaylandSurface::throttlingView() const
{
    Q_D(const QWaylandSurface);
    if (d->views.isEmpty())
        return Q_NULLPTR;
    return d->views.first();
}

void QWaylandSurface::setThrottlingView(QWaylandView *view)
{
    Q_D(QWaylandSurface);

    if (!view)
        return;

    int index = d->views.indexOf(view);

    if (index < 0) {
        view->setSurface(this);
        index = d->views.indexOf(view);
    }

    d->views.move(index, 0);
}

QList<QWaylandView *> QWaylandSurface::views() const
{
    Q_D(const QWaylandSurface);
    return d->views;
}

QWaylandSurface *QWaylandSurface::fromResource(::wl_resource *res)
{
    QWaylandSurfacePrivate *s = QWaylandSurfacePrivate::fromResource(res);
    if (s)
        return s->q_func();
    return Q_NULLPTR;
}

struct wl_resource *QWaylandSurface::resource() const
{
    Q_D(const QWaylandSurface);
    return d->resource()->handle;
}

QWaylandSurfacePrivate *QWaylandSurfacePrivate::get(QWaylandSurface *surface)
{
    return surface ? surface->d_func() : Q_NULLPTR;
}

void QWaylandSurfacePrivate::refView(QWaylandView *view)
{
    Q_Q(QWaylandSurface);
    if (views.contains(view))
        return;

    views.append(view);
    q->ref();
}

void QWaylandSurfacePrivate::derefView(QWaylandView *view)
{
    Q_Q(QWaylandSurface);
    int nViews = views.removeAll(view);

    for (int i = 0; i < nViews && refCount > 0; i++) {
        q->deref();
    }
}

QT_END_NAMESPACE
