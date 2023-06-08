// Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"

#include "wayland_wrapper/qwlbuffermanager_p.h"
#include "wayland_wrapper/qwlregion_p.h"
#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#if QT_CONFIG(wayland_datadevice)
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"
#endif

#include "qwaylandinputmethodcontrol_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandClient>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandBufferRef>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwaylandview_p.h>
#include <QtWaylandCompositor/private/qwaylandseat_p.h>
#include <QtWaylandCompositor/private/qwaylandutils_p.h>

#include <QtCore/private/qobject_p.h>

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtCore/QDebug>
#include <QtCore/QtMath>

QT_BEGIN_NAMESPACE

namespace QtWayland {
class FrameCallback {
public:
    FrameCallback(QWaylandSurface *surf, wl_resource *res)
        : surface(surf)
        , resource(res)
    {
        wl_resource_set_implementation(res, nullptr, this, destroyCallback);
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
        FrameCallback *_this = static_cast<FrameCallback *>(wl_resource_get_user_data(res));
        if (_this->surface)
            QWaylandSurfacePrivate::get(_this->surface)->removeFrameCallback(_this);
        delete _this;
    }
    QWaylandSurface *surface = nullptr;
    wl_resource *resource = nullptr;
    bool canSend = false;
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
    : inputRegion(infiniteRegion())
{
    pending.buffer = QWaylandBufferRef();
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

    for (QtWayland::FrameCallback *c : std::as_const(pendingFrameCallbacks))
        c->destroy();
    for (QtWayland::FrameCallback *c : std::as_const(frameCallbacks))
        c->destroy();
}

void QWaylandSurfacePrivate::removeFrameCallback(QtWayland::FrameCallback *callback)
{
    pendingFrameCallbacks.removeOne(callback);
    frameCallbacks.removeOne(callback);
}

void QWaylandSurfacePrivate::notifyViewsAboutDestruction()
{
    Q_Q(QWaylandSurface);
    const auto viewsCopy = views; // Views will be removed from the list when marked as destroyed
    for (QWaylandView *view : viewsCopy) {
        QWaylandViewPrivate::get(view)->markSurfaceAsDestroyed(q);
    }
    if (hasContent) {
        hasContent = false;
        emit q->hasContentChanged();
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
    pending.buffer = QWaylandBufferRef(getBuffer(buffer));
    pending.offset = QPoint(x, y);
    pending.newlyAttached = true;
}

/*
    Note: The Wayland protocol specifies that buffer scale and damage can be interleaved, so
    we cannot scale the damage region until commit. We assume that clients will either use
    surface_damage or surface_damage_buffer within one frame for one surface.
*/

void QWaylandSurfacePrivate::surface_damage(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    pending.surfaceDamage = pending.surfaceDamage.united(QRect(x, y, width, height));
}

void QWaylandSurfacePrivate::surface_damage_buffer(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    pending.bufferDamage = pending.bufferDamage.united(QRect(x, y, width, height));
}

void QWaylandSurfacePrivate::surface_frame(Resource *resource, uint32_t callback)
{
    Q_Q(QWaylandSurface);
    struct wl_resource *frame_callback = wl_resource_create(resource->client(), &wl_callback_interface, wl_callback_interface.version, callback);
    pendingFrameCallbacks << new QtWayland::FrameCallback(q, frame_callback);
}

void QWaylandSurfacePrivate::surface_set_opaque_region(Resource *, struct wl_resource *region)
{
    pending.opaqueRegion = region ? QtWayland::Region::fromResource(region)->region() : QRegion();
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

    // Needed in order to know whether we want to emit signals later
    QSize oldBufferSize = bufferSize;
    QRectF oldSourceGeometry = sourceGeometry;
    QSize oldDestinationSize = destinationSize;
    bool oldHasContent = hasContent;
    int oldBufferScale = bufferScale;

    // Update all internal state
    if (pending.buffer.hasBuffer() || pending.newlyAttached)
        bufferRef = pending.buffer;
    bufferScale = pending.bufferScale;
    bufferSize = bufferRef.size();
    QSize surfaceSize = bufferSize / bufferScale;
    sourceGeometry = !pending.sourceGeometry.isValid() ? QRect(QPoint(), surfaceSize) : pending.sourceGeometry;
    destinationSize = pending.destinationSize.isEmpty() ? sourceGeometry.size().toSize() : pending.destinationSize;
    QRect destinationRect(QPoint(), destinationSize);
    // pending.damage is already in surface coordinates
    damage = pending.surfaceDamage.intersected(destinationRect);
    if (!pending.bufferDamage.isNull()) {
        if (bufferScale == 1) {
            damage |= pending.bufferDamage.intersected(destinationRect); // Already in surface coordinates
        } else {
            // We must transform pending.damage from buffer coordinate system to surface coordinates
            // TODO(QTBUG-85461): Also support wp_viewport setting more complex transformations
            auto xform = [](const QRect &r, int scale) -> QRect {
                QRect res{
                    QPoint{ r.x() / scale, r.y() / scale },
                    QPoint{ (r.right() + scale - 1) / scale, (r.bottom() + scale - 1) / scale }
                };
                return res;
            };
            for (const QRect &r : pending.bufferDamage)
                damage |= xform(r, bufferScale).intersected(destinationRect);
        }
    }
    hasContent = bufferRef.hasContent();
    frameCallbacks << pendingFrameCallbacks;
    inputRegion = pending.inputRegion.intersected(destinationRect);
    opaqueRegion = pending.opaqueRegion.intersected(destinationRect);
    bool becameOpaque = opaqueRegion.boundingRect().contains(destinationRect);
    if (becameOpaque != isOpaque) {
        isOpaque = becameOpaque;
        emit q->isOpaqueChanged();
    }

    QPoint offsetForNextFrame = pending.offset;

    if (viewport)
        viewport->checkCommittedState();

    // Clear per-commit state
    pending.buffer = QWaylandBufferRef();
    pending.offset = QPoint();
    pending.newlyAttached = false;
    pending.bufferDamage = QRegion();
    pending.surfaceDamage = QRegion();
    pendingFrameCallbacks.clear();

    // Notify buffers and views
    if (auto *buffer = bufferRef.buffer())
        buffer->setCommitted(damage);
    for (auto *view : std::as_const(views))
        view->bufferCommitted(bufferRef, damage);

    // Now all double-buffered state has been applied so it's safe to emit general signals
    // i.e. we won't have inconsistensies such as mismatched surface size and buffer scale in
    // signal handlers.

    emit q->damaged(damage);

    if (oldBufferSize != bufferSize)
        emit q->bufferSizeChanged();

    if (oldBufferScale != bufferScale)
        emit q->bufferScaleChanged();

    if (oldDestinationSize != destinationSize)
        emit q->destinationSizeChanged();

    if (oldSourceGeometry != sourceGeometry)
        emit q->sourceGeometryChanged();

    if (oldHasContent != hasContent)
        emit q->hasContentChanged();

    if (!offsetForNextFrame.isNull())
        emit q->offsetForNextFrame(offsetForNextFrame);

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

QtWayland::ClientBuffer *QWaylandSurfacePrivate::getBuffer(struct ::wl_resource *buffer)
{
    QtWayland::BufferManager *bufMan = QWaylandCompositorPrivate::get(compositor)->bufferManager();
    return bufMan->getBuffer(buffer);
}

/*!
 * \class QWaylandSurfaceRole
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandSurfaceRole class represents the role of the surface in context of wl_surface.
 *
 * QWaylandSurfaceRole is used to represent the role of a QWaylandSurface. According to the protocol
 * specification, the role of a surface is permanent once set, and if the same surface is later
 * reused for a different role, this constitutes a protocol error. Setting the surface to the same
 * role multiple times is not an error.
 *
 * As an example, the QWaylandXdgShell can assign either "popup" or "toplevel" roles to surfaces.
 * If \c get_toplevel is requested on a surface which has previously received a \c get_popup
 * request, then the compositor will issue a protocol error.
 *
 * Roles are compared by pointer value, so any two objects of QWaylandSurfaceRole will be considered
 * different roles, regardless of what their \l{name()}{names} are. A typical way of assigning a
 * role is to have a static QWaylandSurfaceRole object to represent it.
 *
 * \code
 * class MyShellSurfaceSubType
 * {
 *     static QWaylandSurfaceRole s_role;
 *     // ...
 * };
 *
 * // ...
 *
 * surface->setRole(&MyShellSurfaceSubType::s_role, resource->handle, MY_ERROR_CODE);
 * \endcode
 */

/*!
 * \fn QWaylandSurfaceRole::QWaylandSurfaceRole(const QByteArray &name)
 *
 * Creates a QWaylandSurfaceRole and assigns it \a name. The name is used in error messages
 * involving this QWaylandSurfaceRole.
 */

/*!
 * \fn const QByteArray QWaylandSurfaceRole::name()
 *
 * Returns the name of the QWaylandSurfaceRole. The name is used in error messages involving this
 * QWaylandSurfaceRole, for example if an attempt is made to change the role of a surface.
 */

/*!
 * \qmltype WaylandSurface
 * \instantiates QWaylandSurface
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief Represents a rectangular area on an output device.
 *
 * This type encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface \c wl_surface in the Wayland protocol.
 */

/*!
 * \class QWaylandSurface
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandSurface class represents a rectangular area on an output device.
 *
 * This class encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface \c wl_surface in the Wayland protocol.
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
    if (d->compositor)
        QWaylandCompositorPrivate::get(d->compositor)->unregisterSurface(this);
    d->notifyViewsAboutDestruction();
}

/*!
 * \qmlmethod void QtWayland.Compositor::WaylandSurface::initialize(WaylandCompositor compositor, WaylandClient client, int id, int version)
 *
 * Initializes the WaylandSurface with the given \a compositor and \a client, and with the given \a id
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
#if QT_CONFIG(im)
    d->inputMethodControl = new QWaylandInputMethodControl(this);
#endif
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
 * \qmlproperty WaylandClient QtWayland.Compositor::WaylandSurface::client
 *
 * This property holds the client using this WaylandSurface.
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
        return nullptr;

    return d->client;
}

/*!
 * Holds the \c wl_client using this QWaylandSurface.
 */
::wl_client *QWaylandSurface::waylandClient() const
{
    if (auto *c = client())
        return c->client();

    return nullptr;
}

/*!
 * \qmlproperty bool QtWayland.Compositor::WaylandSurface::hasContent
 *
 * This property holds whether the WaylandSurface has content.
 */

/*!
 * \property QWaylandSurface::hasContent
 *
 * This property holds whether the QWaylandSurface has content.
 */
bool QWaylandSurface::hasContent() const
{
    Q_D(const QWaylandSurface);
    return d->hasContent;
}

/*!
 * \qmlproperty rect QtWayland.Compositor::WaylandSurface::sourceGeometry
 * \since 5.13
 *
 * This property describes the portion of the attached Wayland buffer that should
 * be drawn on the screen. The coordinates are from the corner of the buffer and are
 * scaled by \l bufferScale.
 *
 * \sa bufferScale
 * \sa bufferSize
 * \sa destinationSize
 */

/*!
 * \property QWaylandSurface::sourceGeometry
 * \since 5.13
 *
 * This property describes the portion of the attached QWaylandBuffer that should
 * be drawn on the screen. The coordinates are from the corner of the buffer and are
 * scaled by \l bufferScale.
 *
 * \sa bufferScale
 * \sa bufferSize
 * \sa destinationSize
 */
QRectF QWaylandSurface::sourceGeometry() const
{
    Q_D(const QWaylandSurface);
    return d->sourceGeometry;
}

/*!
 * \qmlproperty size QtWayland.Compositor::WaylandSurface::destinationSize
 * \since 5.13
 *
 * This property holds the size of this WaylandSurface in surface coordinates.
 *
 * \sa bufferScale
 * \sa bufferSize
 */

/*!
 * \property QWaylandSurface::destinationSize
 * \since 5.13
 *
 * This property holds the size of this WaylandSurface in surface coordinates.
 *
 * \sa bufferScale
 * \sa bufferSize
 */
QSize QWaylandSurface::destinationSize() const
{
    Q_D(const QWaylandSurface);
    return d->destinationSize;
}

/*!
 * \qmlproperty size QtWayland.Compositor::WaylandSurface::bufferSize
 *
 * This property holds the size of the current buffer of this WaylandSurface in pixels,
 * not in surface coordinates.
 *
 * For the size in surface coordinates, use \l destinationSize instead.
 *
 * \sa destinationSize
 * \sa bufferScale
 */

/*!
 * \property QWaylandSurface::bufferSize
 *
 * This property holds the size of the current buffer of this QWaylandSurface in pixels,
 * not in surface coordinates.
 *
 * For the size in surface coordinates, use \l destinationSize instead.
 *
 * \sa destinationSize
 * \sa bufferScale
 */
QSize QWaylandSurface::bufferSize() const
{
    Q_D(const QWaylandSurface);
    return d->bufferSize;
}

/*!
 * \qmlproperty size QtWayland.Compositor::WaylandSurface::bufferScale
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
 * \qmlproperty enum QtWayland.Compositor::WaylandSurface::contentOrientation
 *
 * This property holds the orientation of the WaylandSurface's contents.
 *
 * \sa {WaylandOutput::transform}{WaylandOutput.transform}
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
 * \qmlproperty enum QtWayland.Compositor::WaylandSurface::origin
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
    return d->bufferRef.origin();
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
    for (QtWayland::FrameCallback *c : std::as_const(d->frameCallbacks))
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
            d->frameCallbacks.at(i)->surface = nullptr;
            d->frameCallbacks.at(i)->send(time);
            d->frameCallbacks.removeAt(i);
        } else {
            i++;
        }
    }
}

/*!
 * Returns \c true if the QWaylandSurface's input region contains the point \a p.
 * Otherwise returns \c false.
 */
bool QWaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const QWaylandSurface);
    return d->inputRegion.contains(p);
}

/*!
 * Returns \c true if the QWaylandSurface's input region contains the point \a position.
 * Otherwise returns \c false.
 *
 * \since 5.14
 */
bool QWaylandSurface::inputRegionContains(const QPointF &position) const
{
    Q_D(const QWaylandSurface);
    // QRegion::contains operates in integers. If a region has a rect (0,0,10,10), (0,0) is
    // inside while (10,10) is outside. Therefore, we can't use QPoint::toPoint(), which will
    // round upwards, meaning the point (-0.25,-0.25) would be rounded to (0,0) and count as
    // being inside the region, and similarly, a point (9.75,9.75) inside the region would be
    // rounded upwards and count as being outside the region.
    const QPoint floored(qFloor(position.x()), qFloor(position.y()));
    return d->inputRegion.contains(floored);
}

/*!
 * \qmlmethod void QtWayland.Compositor::WaylandSurface::destroy()
 *
 * Destroys the WaylandSurface.
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
 * \qmlmethod bool QtWayland.Compositor::WaylandSurface::isDestroyed()
 *
 * Returns \c true if the WaylandSurface has been destroyed. Otherwise returns \c false.
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
 * \qmlproperty bool QtWayland.Compositor::WaylandSurface::cursorSurface
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
    if (d->isCursorSurface == cursorSurface)
        return;

    d->isCursorSurface = cursorSurface;
    emit cursorSurfaceChanged();
}

bool QWaylandSurface::isCursorSurface() const
{
    Q_D(const QWaylandSurface);
    return d->isCursorSurface;
}

/*!
 * \qmlproperty bool QtWayland.Compositor::WaylandSurface::inhibitsIdle
 * \since 5.14
 *
 * This property holds whether this surface is intended to inhibit the idle
 * behavior of the compositor such as screen blanking, locking and screen saving.
 *
 * \sa IdleInhibitManagerV1
 */

/*!
 * \property QWaylandSurface::inhibitsIdle
 * \since 5.14
 *
 * This property holds whether this surface is intended to inhibit the idle
 * behavior of the compositor such as screen blanking, locking and screen saving.
 *
 * \sa QWaylandIdleInhibitManagerV1
 */
bool QWaylandSurface::inhibitsIdle() const
{
    Q_D(const QWaylandSurface);
    return !d->idleInhibitors.isEmpty();
}

/*!
 *  \qmlproperty bool QtWayland.Compositor::WaylandSurface::isOpaque
 *  \since 6.4
 *
 *  This property holds whether the surface is fully opaque, as reported by the
 *  client through the set_opaque_region request.
 */

/*!
 *  \property QWaylandSurface::isOpaque
 *  \since 6.4
 *
 *  This property holds whether the surface is fully opaque, as reported by the
 *  client through the set_opaque_region request.
 */
bool QWaylandSurface::isOpaque() const
{
    Q_D(const QWaylandSurface);
    return d->isOpaque;
}

#if QT_CONFIG(im)
QWaylandInputMethodControl *QWaylandSurface::inputMethodControl() const
{
    Q_D(const QWaylandSurface);
    return d->inputMethodControl;
}
#endif

/*!
 * Updates the surface with the compositor's retained clipboard selection. Although
 * this is done automatically when the surface receives keyboard focus, this
 * function is useful for updating clients which do not have keyboard focus.
 */
#if QT_CONFIG(clipboard)
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    QWaylandSeat *seat = d->compositor->defaultSeat();
    if (seat) {
        const QtWayland::DataDevice *dataDevice = QWaylandSeatPrivate::get(seat)->dataDevice();
        if (dataDevice) {
            QWaylandCompositorPrivate::get(d->compositor)->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}
#endif

/*!
 * Returns this QWaylandSurface's primary view.
 *
 * \sa QWaylandView::advance(), QWaylandSurface::setPrimaryView()
 */
QWaylandView *QWaylandSurface::primaryView() const
{
    Q_D(const QWaylandSurface);
    if (d->views.isEmpty())
        return nullptr;
    return d->views.first();
}

/*!
 * Sets this QWaylandSurface's primary view to \a view, in case there are
 * multiple views of this surface. The primary view is the view that
 * governs the client's refresh rate. It takes care of discarding buffer
 * references when QWaylandView::advance() is called. See the documentation
 * for QWaylandView::advance() for more details.
 *
 * In shell surface integrations, such as QWaylandWlShellIntegration and
 * QWaylandXdgShellV5Integration, maximize and fullscreen requests from the
 * client will only have an effect if the integration has the primary view
 * of the surface.
 *
 * \sa QWaylandView::advance()
 */
void QWaylandSurface::setPrimaryView(QWaylandView *view)
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
 * Returns the QWaylandSurface corresponding to the Wayland resource \a resource.
 */
QWaylandSurface *QWaylandSurface::fromResource(::wl_resource *resource)
{
    if (auto p = QtWayland::fromResource<QWaylandSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
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
 * Sets a \a role on the surface. A role defines how a surface will be mapped on screen; without a
 * role a surface is supposed to be hidden. Once a role is assigned to a surface, this becomes its
 * permanent role. Any subsequent call to \c setRole() with a different role will trigger a
 * protocol error to the \a errorResource and send an \a errorCode to the client. Enforcing this
 * requirement is the main purpose of the surface role.
 *
 * The \a role is compared by pointer value. Any two objects of QWaylandSurfaceRole will be
 * considered different roles, regardless of their names.
 *
 * The surface role is set internally by protocol implementations when a surface is adopted for a
 * specific purpose, for example in a \l{Shell Extensions - Qt Wayland Compositor}{shell extension}.
 * Unless you are developing extensions which use surfaces in this way, you should not call this
 * function.
 *
 * Returns true if a role can be assigned; false otherwise.
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
    return surface ? surface->d_func() : nullptr;
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
    view->bufferCommitted(bufferRef, QRect(QPoint(0,0), bufferRef.size()));
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
    QWaylandSurface *oldParent = nullptr; // TODO: implement support for switching parents

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

/*!
 * \qmlsignal QtWayland.Compositor::WaylandSurface::childAdded(WaylandSurface child)
 *
 * This signal is emitted when a wl_subsurface, \a child, has been added to the surface.
 */

/*!
 * \fn void QWaylandSurface::childAdded(QWaylandSurface *child)
 *
 * This signal is emitted when a wl_subsurface, \a child, has been added to the surface.
 */

/*!
 * \qmlsignal QtWayland.Compositor::WaylandSurface::surfaceDestroyed()
 *
 * This signal is emitted when the corresponding wl_surface is destroyed.
 */

/*!
 * \fn void QWaylandSurface::surfaceDestroyed()
 *
 * This signal is emitted when the corresponing wl_surface is destroyed.
 */

/*!
 * \qmlsignal void QtWayland.Compositor::WaylandSurface::dragStarted(WaylandDrag drag)
 *
 * This signal is emitted when a \a drag has started from this surface.
 */

/*!
 * \fn void QWaylandSurface::dragStarted(QWaylandDrag *drag)
 *
 * This signal is emitted when a \a drag has started from this surface.
 */

/*!
 * \fn void damaged(const QRegion &rect)
 *
 * This signal is emitted when the client tells the compositor that a particular part of, or
 * possibly the entire surface has been updated, so the compositor can redraw that part.
 *
 * While the compositor APIs take care of redrawing automatically, this function may be useful
 * if you require a specific, custom behavior.
 */

/*!
 * \fn void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
 *
 * This signal is emitted when the client has requested that this surface should be a
 * subsurface of \a newParent.
 */

QT_END_NAMESPACE

#include "moc_qwaylandsurface.cpp"
