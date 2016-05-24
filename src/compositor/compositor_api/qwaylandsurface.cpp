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
#include "qwaylandinputmethodcontrol_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandClient>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandBufferRef>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwaylandview_p.h>
#include <QtWaylandCompositor/private/qwaylandinput_p.h>

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
        if (_this->surface)
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

#ifndef QT_NO_DEBUG
QList<QWaylandSurfacePrivate *> QWaylandSurfacePrivate::uninitializedSurfaces;
#endif

QWaylandSurfacePrivate::QWaylandSurfacePrivate()
    : QtWaylandServer::wl_surface()
    , compositor(Q_NULLPTR)
    , refCount(1)
    , client(Q_NULLPTR)
    , buffer(0)
    , role(0)
    , inputRegion(infiniteRegion())
    , bufferScale(1)
    , isCursorSurface(false)
    , destroyed(false)
    , mapped(false)
    , isInitialized(false)
    , contentOrientation(Qt::PrimaryOrientation)
    , inputMethodControl(Q_NULLPTR)
    , subsurface(0)
{
    pending.buffer = 0;
    pending.newlyAttached = false;
    pending.inputRegion = infiniteRegion();
    pending.bufferScale = 1;
#ifndef QT_NO_DEBUG
    addUninitializedSurface(this);
#endif
}

QWaylandSurfacePrivate::~QWaylandSurfacePrivate()
{
    for (int i = 0; i < views.size(); i++) {
        QWaylandViewPrivate::get(views.at(i))->markSurfaceAsDestroyed(q_func());
    }
    views.clear();

    bufferRef = QWaylandBufferRef();

    for (int i = 0; i < bufferPool.size(); i++)
        bufferPool[i]->setDestroyIfUnused(true);

    foreach (QtWayland::FrameCallback *c, pendingFrameCallbacks)
        c->destroy();
    foreach (QtWayland::FrameCallback *c, frameCallbacks)
        c->destroy();
}

void QWaylandSurfacePrivate::setSize(const QSize &s)
{
    Q_Q(QWaylandSurface);
    if (size != s) {
        opaqueRegion = QRegion();
        size = s;
        q->sizeChanged();
    }
}

void QWaylandSurfacePrivate::setBufferScale(int scale)
{
    Q_Q(QWaylandSurface);
    if (scale == bufferScale)
        return;
    bufferScale = scale;
    emit q->bufferScaleChanged();
}

void QWaylandSurfacePrivate::removeFrameCallback(QtWayland::FrameCallback *callback)
{
    pendingFrameCallbacks.removeOne(callback);
    frameCallbacks.removeOne(callback);
}

void QWaylandSurfacePrivate::notifyViewsAboutDestruction()
{
    Q_Q(QWaylandSurface);
    foreach (QWaylandView *view, views) {
        QWaylandViewPrivate::get(view)->markSurfaceAsDestroyed(q);
    }
    if (mapped) {
        mapped = false;
        emit q->mappedChanged();
    }
}

#ifndef QT_NO_DEBUG
void QWaylandSurfacePrivate::addUninitializedSurface(QWaylandSurfacePrivate *surface)
{
    Q_ASSERT(!surface->isInitialized);
    Q_ASSERT(!uninitializedSurfaces.contains(surface));
    uninitializedSurfaces.append(surface);
}

void QWaylandSurfacePrivate::removeUninitializedSurface(QWaylandSurfacePrivate *surface)
{
    Q_ASSERT(surface->isInitialized);
    bool removed = uninitializedSurfaces.removeOne(surface);
    Q_ASSERT(removed);
}

bool QWaylandSurfacePrivate::hasUninitializedSurface()
{
    return uninitializedSurfaces.size();
}
#endif

void QWaylandSurfacePrivate::surface_destroy_resource(Resource *)
{
    Q_Q(QWaylandSurface);
    notifyViewsAboutDestruction();

    destroyed = true;
    emit q->surfaceDestroyed();
    q->destroy();
}

void QWaylandSurfacePrivate::surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandSurfacePrivate::surface_attach(Resource *, struct wl_resource *buffer, int x, int y)
{
    if (pending.buffer)
        pending.buffer->disown();
    pending.buffer = createSurfaceBuffer(buffer);
    pending.offset = QPoint(x, y);
    pending.newlyAttached = true;
}

void QWaylandSurfacePrivate::surface_damage(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    pending.damage = pending.damage.united(QRect(x, y, width, height));
}

void QWaylandSurfacePrivate::surface_frame(Resource *resource, uint32_t callback)
{
    Q_Q(QWaylandSurface);
    struct wl_resource *frame_callback = wl_resource_create(resource->client(), &wl_callback_interface, wl_callback_interface.version, callback);
    pendingFrameCallbacks << new QtWayland::FrameCallback(q, frame_callback);
}

void QWaylandSurfacePrivate::surface_set_opaque_region(Resource *, struct wl_resource *region)
{
    opaqueRegion = region ? QtWayland::Region::fromResource(region)->region() : QRegion();
}

void QWaylandSurfacePrivate::surface_set_input_region(Resource *, struct wl_resource *region)
{
    if (region) {
        pending.inputRegion = QtWayland::Region::fromResource(region)->region();
    } else {
        pending.inputRegion = infiniteRegion();
    }
}

void QWaylandSurfacePrivate::surface_commit(Resource *)
{
    Q_Q(QWaylandSurface);

    if (pending.buffer || pending.newlyAttached) {
        setBackBuffer(pending.buffer, pending.damage);
    }

    pending.buffer = 0;
    pending.offset = QPoint();
    pending.newlyAttached = false;
    pending.damage = QRegion();

    setBufferScale(pending.bufferScale);

    if (buffer)
        buffer->setCommitted();

    frameCallbacks << pendingFrameCallbacks;
    pendingFrameCallbacks.clear();

    inputRegion = pending.inputRegion.intersected(QRect(QPoint(), size));

    emit q->redraw();
}

void QWaylandSurfacePrivate::surface_set_buffer_transform(Resource *resource, int32_t orientation)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandSurface);
    QScreen *screen = QGuiApplication::primaryScreen();
    bool isPortrait = screen->primaryOrientation() == Qt::PortraitOrientation;
    Qt::ScreenOrientation oldOrientation = contentOrientation;
    switch (orientation) {
        case WL_OUTPUT_TRANSFORM_90:
            contentOrientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_180:
            contentOrientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_270:
            contentOrientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        default:
            contentOrientation = Qt::PrimaryOrientation;
    }
    if (contentOrientation != oldOrientation)
        emit q->contentOrientationChanged();
}

void QWaylandSurfacePrivate::surface_set_buffer_scale(QtWaylandServer::wl_surface::Resource *resource, int32_t scale)
{
    Q_UNUSED(resource);
    pending.bufferScale = scale;
}

void QWaylandSurfacePrivate::setBackBuffer(QtWayland::SurfaceBuffer *b, const QRegion &d)
{
    Q_Q(QWaylandSurface);
    buffer = b;

    bufferRef = QWaylandBufferRef(buffer);

    setSize(bufferRef.size());
    damage = d.intersected(QRect(QPoint(), size));

    for (int i = 0; i < views.size(); i++) {
        views.at(i)->attach(bufferRef, damage);
    }

    emit q->damaged(damage);

    bool oldMapped = mapped;
    mapped = QtWayland::SurfaceBuffer::hasContent(buffer);
    if (oldMapped != mapped)
        emit q->mappedChanged();

    if (!pending.offset.isNull())
        emit q->offsetForNextFrame(pending.offset);
}

QtWayland::SurfaceBuffer *QWaylandSurfacePrivate::createSurfaceBuffer(struct ::wl_resource *buffer)
{
    Q_Q(QWaylandSurface);
    QtWayland::SurfaceBuffer *newBuffer = 0;
    for (int i = 0; i < bufferPool.size(); i++) {
        if (!bufferPool[i]->isRegisteredWithBuffer()) {
            newBuffer = bufferPool[i];
            newBuffer->initialize(buffer);
            break;
        }
    }

    if (!newBuffer) {
        newBuffer = new QtWayland::SurfaceBuffer(q);
        newBuffer->initialize(buffer);
        bufferPool.append(newBuffer);
        if (bufferPool.size() > 3)
            qWarning() << "Increased buffer pool size to" << bufferPool.size() << "for surface" << q;
    }

    return newBuffer;
}

/*!
 * \qmltype WaylandSurface
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief A rectangular area which is displayed on an output device.
 *
 * This type encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface wl_surface in the Wayland protocol.
 */

/*!
 * \class QWaylandSurface
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief A rectangular area which is displayed on an output device.
 *
 * This class encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface wl_surface in the Wayland protocol.
 */

/*!
 * Constructs a an uninitialized QWaylandSurface.
 */
QWaylandSurface::QWaylandSurface()
    : QWaylandObject(*new QWaylandSurfacePrivate())
{
}

/*!
 * Constructs and initializes a QWaylandSurface for the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */
QWaylandSurface::QWaylandSurface(QWaylandCompositor *compositor, QWaylandClient *client, uint id, int version)
    : QWaylandObject(*new QWaylandSurfacePrivate())
{
    initialize(compositor, client, id, version);
}

/*!
 * \internal
 */
QWaylandSurface::QWaylandSurface(QWaylandSurfacePrivate &dptr)
    : QWaylandObject(dptr)
{
}

/*!
 * Destroys the QWaylandSurface.
 */
QWaylandSurface::~QWaylandSurface()
{
    Q_D(QWaylandSurface);
    QWaylandCompositorPrivate::get(d->compositor)->unregisterSurface(this);
    d->notifyViewsAboutDestruction();
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSurface::initialize(object compositor, object client, int id, int version)
 *
 * Initializes the QWaylandSurface with the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */

/*!
 * Initializes the QWaylandSurface with the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */
void QWaylandSurface::initialize(QWaylandCompositor *compositor, QWaylandClient *client, uint id, int version)
{
    Q_D(QWaylandSurface);
    d->compositor = compositor;
    d->client = client;
    d->init(client->client(), id, version);
    d->isInitialized = true;
    d->inputMethodControl = new QWaylandInputMethodControl(this);
#ifndef QT_NO_DEBUG
    QWaylandSurfacePrivate::removeUninitializedSurface(d);
#endif
}

/*!
 * Returns true if the QWaylandSurface has been initialized.
 */
bool QWaylandSurface::isInitialized() const
{
    Q_D(const QWaylandSurface);
    return d->isInitialized;
}

/*!
 * \qmlproperty object QtWaylandCompositor::WaylandSurface::client
 *
 * This property holds the client using this QWaylandSurface.
 */

/*!
 * \property QWaylandSurface::client
 *
 * This property holds the client using this QWaylandSurface.
 */
QWaylandClient *QWaylandSurface::client() const
{
    Q_D(const QWaylandSurface);
    if (isDestroyed() || !compositor() || !compositor()->clients().contains(d->client))
        return Q_NULLPTR;

    return d->client;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandSurface::isMapped
 *
 * This property holds whether the WaylandSurface has content.
 */

/*!
 * \property QWaylandSurface::isMapped
 *
 * This property holds whether the QWaylandSurface has content.
 */
bool QWaylandSurface::isMapped() const
{
    Q_D(const QWaylandSurface);
    return d->mapped;
}

/*!
 * \qmlproperty size QtWaylandCompositor::WaylandSurface::size
 *
 * This property holds the WaylandSurface's size in pixels.
 */

/*!
 * \property QWaylandSurface::size
 *
 * This property holds the QWaylandSurface's size in pixels.
 */
QSize QWaylandSurface::size() const
{
    Q_D(const QWaylandSurface);
    return d->size;
}

/*!
 * \qmlproperty size QtWaylandCompositor::WaylandSurface::bufferScale
 *
 * This property holds the WaylandSurface's buffer scale. The buffer scale lets
 * a client supply higher resolution buffer data for use on high resolution
 * outputs.
 */

/*!
 * \property QWaylandSurface::bufferScale
 *
 * This property holds the QWaylandSurface's buffer scale. The buffer scale
 * lets a client supply higher resolution buffer data for use on high
 * resolution outputs.
 */
int QWaylandSurface::bufferScale() const
{
    Q_D(const QWaylandSurface);
    return d->bufferScale;
}

/*!
 * \qmlproperty enum QtWaylandCompositor::WaylandSurface::contentOrientation
 *
 * This property holds the orientation of the WaylandSurface's contents.
 *
 * \sa QtWaylandCompositor::WaylandOutput::transform
 */

/*!
 * \property QWaylandSurface::contentOrientation
 *
 * This property holds the orientation of the QWaylandSurface's contents.
 *
 * \sa QWaylandOutput::transform
 */
Qt::ScreenOrientation QWaylandSurface::contentOrientation() const
{
    Q_D(const QWaylandSurface);
    return d->contentOrientation;
}

/*!
 * \enum QWaylandSurface::Origin
 *
 * This enum type is used to specify the origin of a QWaylandSurface's buffer.
 *
 * \value OriginTopLeft The origin is the top left corner of the buffer.
 * \value OriginBottomLeft The origin is the bottom left corner of the buffer.
 */

/*!
 * \qmlproperty enum QtWaylandCompositor::WaylandSurface::origin
 *
 * This property holds the origin of the WaylandSurface's buffer, or
 * WaylandSurface.OriginTopLeft if the surface has no buffer.
 *
 * It can have the following values:
 * \list
 * \li WaylandSurface.OriginTopLeft The origin is the top left corner of the buffer.
 * \li WaylandSurface.OriginBottomLeft The origin is the bottom left corner of the buffer.
 * \endlist
 */

/*!
 * \property QWaylandSurface::origin
 *
 * This property holds the origin of the QWaylandSurface's buffer, or
 * QWaylandSurface::OriginTopLeft if the surface has no buffer.
 */
QWaylandSurface::Origin QWaylandSurface::origin() const
{
    Q_D(const QWaylandSurface);
    return d->buffer ? d->buffer->origin() : QWaylandSurface::OriginTopLeft;
}

/*!
 * Returns the compositor for this QWaylandSurface.
 */
QWaylandCompositor *QWaylandSurface::compositor() const
{
    Q_D(const QWaylandSurface);
    return d->compositor;
}

/*!
 * Prepares all frame callbacks for sending.
 */
void QWaylandSurface::frameStarted()
{
    Q_D(QWaylandSurface);
    foreach (QtWayland::FrameCallback *c, d->frameCallbacks)
        c->canSend = true;
}

/*!
 * Sends pending frame callbacks.
 */
void QWaylandSurface::sendFrameCallbacks()
{
    Q_D(QWaylandSurface);
    uint time = d->compositor->currentTimeMsecs();
    int i = 0;
    while (i < d->frameCallbacks.size()) {
        if (d->frameCallbacks.at(i)->canSend) {
            d->frameCallbacks.at(i)->surface = Q_NULLPTR;
            d->frameCallbacks.at(i)->send(time);
            d->frameCallbacks.removeAt(i);
        } else {
            i++;
        }
    }
}

/*!
 * Returns true if the QWaylandSurface's input region contains the point \a p.
 * Otherwise returns false.
 */
bool QWaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const QWaylandSurface);
    return d->inputRegion.contains(p);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSurface::destroy()
 *
 * Destroys the QWaylandSurface.
 */

/*!
 * Destroys the QWaylandSurface.
 */
void QWaylandSurface::destroy()
{
    Q_D(QWaylandSurface);
    d->deref();
}

/*!
 * \qmlmethod bool QtWaylandCompositor::WaylandSurface::isDestroyed()
 *
 * Returns true if the WaylandSurface has been destroyed. Otherwise returns false.
 */

/*!
 * Returns true if the QWaylandSurface has been destroyed. Otherwise returns false.
 */
bool QWaylandSurface::isDestroyed() const
{
    Q_D(const QWaylandSurface);
    return d->destroyed;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandSurface::cursorSurface
 *
 * This property holds whether the WaylandSurface is a cursor surface.
 */

/*!
 * \property QWaylandSurface::cursorSurface
 *
 * This property holds whether the QWaylandSurface is a cursor surface.
 */
void QWaylandSurface::markAsCursorSurface(bool cursorSurface)
{
    Q_D(QWaylandSurface);
    d->isCursorSurface = cursorSurface;
}

bool QWaylandSurface::isCursorSurface() const
{
    Q_D(const QWaylandSurface);
    return d->isCursorSurface;
}

QWaylandInputMethodControl *QWaylandSurface::inputMethodControl() const
{
    Q_D(const QWaylandSurface);
    return d->inputMethodControl;
}

/*!
 * Updates the surface with the compositor's retained clipboard selection. While this
 * is done automatically when the surface receives keyboard focus, this function is
 * useful for updating clients which do not have keyboard focus.
 */
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    QWaylandInputDevice *inputDevice = d->compositor->defaultInputDevice();
    if (inputDevice) {
        const QtWayland::DataDevice *dataDevice = QWaylandInputDevicePrivate::get(inputDevice)->dataDevice();
        if (dataDevice) {
            QWaylandCompositorPrivate::get(d->compositor)->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}

/*!
 * Returns this QWaylandSurface's throttling view.
 *
 * \sa QWaylandView::advance()
 */
QWaylandView *QWaylandSurface::throttlingView() const
{
    Q_D(const QWaylandSurface);
    if (d->views.isEmpty())
        return Q_NULLPTR;
    return d->views.first();
}

/*!
 * Sets this QWaylandSurface's throttling view to \a view, in case there are
 * multiple views of this surface. The throttling view is the view that
 * governs the client's refresh rate. It takes care of discarding buffer
 * references when QWaylandView::advance() is called. See the documentation
 * for QWaylandView::advance() for more details.
 *
 * \sa QWaylandView::advance()
 */
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

/*!
 * Returns the views for this QWaylandSurface.
 */
QList<QWaylandView *> QWaylandSurface::views() const
{
    Q_D(const QWaylandSurface);
    return d->views;
}

/*!
 * Returns the QWaylandSurface corresponding to the Wayland resource \a res.
 */
QWaylandSurface *QWaylandSurface::fromResource(::wl_resource *res)
{
    return static_cast<QWaylandSurfacePrivate *>(QWaylandSurfacePrivate::Resource::fromResource(res)->surface_object)->q_func();
}

/*!
 * Returns the Wayland resource corresponding to this QWaylandSurface.
 */
struct wl_resource *QWaylandSurface::resource() const
{
    Q_D(const QWaylandSurface);
    return d->resource()->handle;
}

/*!
 * Sets a role on the surface. A role defines how a surface will be mapped on screen, without a role
 * a surface is supposed to be hidden. Only one role at all times can be set on a surface. Attempting
 * to change the role of a surface will trigger a protocol error to the client, while setting the same
 * role many times is allowed.
 *
 * \param errorResource The resource the error will be sent to if the role is being changed.
 * \param errorCode The error code that will be sent to the client.
 */
bool QWaylandSurface::setRole(QWaylandSurfaceRole *role, wl_resource *errorResource, uint32_t errorCode)
{
    Q_D(QWaylandSurface);

    if (d->role && d->role != role) {
            wl_resource_post_error(errorResource, errorCode,
                                   "Cannot assign role %s to wl_surface@%d, already has role %s\n",
                                   role->name().constData(), wl_resource_get_id(resource()),
                                   d->role->name().constData());
            return false;
    }

    d->role = role;
    return true;
}

QWaylandSurfaceRole *QWaylandSurface::role() const
{
    Q_D(const QWaylandSurface);
    return d->role;
}

QWaylandSurfacePrivate *QWaylandSurfacePrivate::get(QWaylandSurface *surface)
{
    return surface ? surface->d_func() : Q_NULLPTR;
}

void QWaylandSurfacePrivate::ref()
{
    ++refCount;
}

void QWaylandSurfacePrivate::deref()
{
    if (--refCount == 0)
        QWaylandCompositorPrivate::get(compositor)->destroySurface(q_func());
}

void QWaylandSurfacePrivate::refView(QWaylandView *view)
{
    if (views.contains(view))
        return;

    views.append(view);
    ref();
    QWaylandBufferRef ref(buffer);
    view->attach(ref, QRect(QPoint(0,0), ref.size()));
}

void QWaylandSurfacePrivate::derefView(QWaylandView *view)
{
    int nViews = views.removeAll(view);

    for (int i = 0; i < nViews && refCount > 0; i++) {
        deref();
    }
}

void QWaylandSurfacePrivate::initSubsurface(QWaylandSurface *parent, wl_client *client, int id, int version)
{
    Q_Q(QWaylandSurface);
    QWaylandSurface *oldParent = 0; // TODO: implement support for switching parents

    subsurface = new Subsurface(this);
    subsurface->init(client, id, version);
    subsurface->parentSurface = parent->d_func();
    emit q->parentChanged(parent, oldParent);
    emit parent->childAdded(q);
}

void QWaylandSurfacePrivate::Subsurface::subsurface_set_position(wl_subsurface::Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    position = QPoint(x,y);
    emit surface->q_func()->subsurfacePositionChanged(position);

}

void QWaylandSurfacePrivate::Subsurface::subsurface_place_above(wl_subsurface::Resource *resource, struct wl_resource *sibling)
{
    Q_UNUSED(resource);
    emit surface->q_func()->subsurfacePlaceAbove(QWaylandSurface::fromResource(sibling));
}

void QWaylandSurfacePrivate::Subsurface::subsurface_place_below(wl_subsurface::Resource *resource, struct wl_resource *sibling)
{
    Q_UNUSED(resource);
    emit surface->q_func()->subsurfacePlaceBelow(QWaylandSurface::fromResource(sibling));
}

void QWaylandSurfacePrivate::Subsurface::subsurface_set_sync(wl_subsurface::Resource *resource)
{
    Q_UNUSED(resource);
    // TODO: sync/desync implementation
    qDebug() << Q_FUNC_INFO;
}

void QWaylandSurfacePrivate::Subsurface::subsurface_set_desync(wl_subsurface::Resource *resource)
{
    Q_UNUSED(resource);
    // TODO: sync/desync implementation
    qDebug() << Q_FUNC_INFO;
}

QT_END_NAMESPACE
