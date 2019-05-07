/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qwaylandwlscaler_p.h"

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

#if QT_DEPRECATED_SINCE(5, 13)
/*!
    \qmltype WlScaler
    \inqmlmodule QtWayland.Compositor
    \since 5.13
    \brief Provides an extension for surface resizing and cropping.

    The WlScaler extension provides a way for clients to resize and crop surface contents.

    WlScaler corresponds to the Wayland interface, \c wl_scaler.

    \c wl_scaler is a non-standard and deprecated protocol that has largely been replaced by
    \c wp_viewporter. I.e. This extensions is only useful for supporting legacy clients.
    \c wp_viewporter support is enabled automatically for all Qml compositors.

    To provide the functionality of the extension in a compositor, create an instance of the
    WlScaler component and add it to the list of extensions supported by the compositor:

    \qml \QtMinorVersion
    import QtWayland.Compositor 1.\1

    WaylandCompositor {
        // ...
        WlScaler {}
    }
    \endqml

    \deprecated
*/

/*!
    \class QWaylandWlScaler
    \inmodule QtWaylandCompositor
    \since 5.13
    \brief Provides an extension for surface resizing and croping.

    The QWaylandWlScaler extension provides a way for clients to resize and crop surface
    contents.

    QWaylandWlScaler corresponds to the Wayland interface, \c wl_scaler.

    \c wl_scaler is a non-standard and deprecated protocol that has largely been replaced by
    \c wp_viewporter. I.e. This extensions is only useful for supporting legacy clients.

    \sa QWaylandViewporter

    \deprecated
*/

/*!
    Constructs a QWaylandWlScaler object.
*/
QWaylandWlScaler::QWaylandWlScaler()
    : QWaylandCompositorExtensionTemplate<QWaylandWlScaler>(*new QWaylandWlScalerPrivate)
{
}

/*!
 * Constructs a QWaylandWlScaler object for the provided \a compositor.
 */
QWaylandWlScaler::QWaylandWlScaler(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandWlScaler>(compositor, *new QWaylandWlScalerPrivate())
{
}

/*!
    Initializes the extension.
*/
void QWaylandWlScaler::initialize()
{
    Q_D(QWaylandWlScaler);

    QWaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandWlScaler";
        return;
    }
    d->init(compositor->display(), 2);
}

/*!
    Returns the Wayland interface for the QWaylandWlScaler.
*/
const wl_interface *QWaylandWlScaler::interface()
{
    return QWaylandWlScalerPrivate::interface();
}

void QWaylandWlScalerPrivate::scaler_destroy(Resource *resource)
{
    // Viewport objects are allowed ot outlive the scaler
    wl_resource_destroy(resource->handle);
}

void QWaylandWlScalerPrivate::scaler_get_viewport(Resource *resource, uint id, wl_resource *surfaceResource)
{
    auto *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qWarning() << "Couldn't find surface for viewporter";
        return;
    }

    // Note: This will only protect us not creating scalers for surfaces with wp_viewport objects
    auto *surfacePrivate = QWaylandSurfacePrivate::get(surface);
    if (surfacePrivate->viewport) {
        wl_resource_post_error(resource->handle, WL_SCALER_ERROR_VIEWPORT_EXISTS,
                               "viewport already exists for surface");
        return;
    }

    // We can't set viewport here, since it's of the new type for wp_viewporter
//    surfacePrivate->viewport = new Viewport(surface, resource->client(), id, resource->version());
    new Viewport(surface, resource->client(), id, resource->version());
}

QWaylandWlScalerPrivate::Viewport::Viewport(QWaylandSurface *surface, wl_client *client, int id, int version)
    : QtWaylandServer::wl_viewport(client, id, version)
    , m_surface(surface)
{
    Q_ASSERT(surface);
}

//TODO: This isn't currently called
// This function has to be called immediately after a surface is committed, before no
// other client events have been dispatched, or we may incorrectly error out on an
// incomplete pending state. See comment below.
void QWaylandWlScalerPrivate::Viewport::checkCommittedState()
{
    auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);

    // We can't use the current state for destination/source when checking,
    // as that has fallbacks to the buffer size so we can't distinguish
    // between the set/unset case. We use the pending state because no other
    // requests has modified it yet.
    QSize destination = surfacePrivate->pending.destinationSize;
    QRectF source = surfacePrivate->pending.sourceGeometry;

    if (!destination.isValid() && source.size() != source.size().toSize()) {
        //TODO: Do rounding to nearest integer
    }

    QRectF max = QRectF(QPointF(), m_surface->bufferSize() / m_surface->bufferScale());
    // We can't use QRectF.contains, because that would return false for values on the border
    if (max.united(source) != max) {
        //TODO: surface contents are no undefined, surface size is still valid though
        qCDebug(qLcWaylandCompositor) << "Source set outside buffer bounds (client error)";
    }
}


void QWaylandWlScalerPrivate::Viewport::viewport_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandWlScalerPrivate::Viewport::viewport_destroy(Resource *resource)
{
    if (m_surface) {
        auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
        surfacePrivate->pending.destinationSize = QSize();
        surfacePrivate->pending.sourceGeometry = QRectF();
    }
    wl_resource_destroy(resource->handle);
}

void QWaylandWlScalerPrivate::Viewport::viewport_set(QtWaylandServer::wl_viewport::Resource *resource, wl_fixed_t src_x, wl_fixed_t src_y, wl_fixed_t src_width, wl_fixed_t src_height, int32_t dst_width, int32_t dst_height)
{
    viewport_set_source(resource, src_x, src_y, src_width, src_height);
    viewport_set_destination(resource, dst_width, dst_height);
}

void QWaylandWlScalerPrivate::Viewport::viewport_set_source(QtWaylandServer::wl_viewport::Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        qCDebug(qLcWaylandCompositor) << "set_source requested for destroyed surface";
        return;
    }

    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    QSizeF size(wl_fixed_to_double(width), wl_fixed_to_double(height));
    QRectF sourceGeometry(position, size);

    if (sourceGeometry == QRectF(-1, -1, -1, -1)) {
        auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
        surfacePrivate->pending.sourceGeometry = QRectF();
        return;
    }

    if (position.x() < 0 || position.y() < 0) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative position in set_source");
        return;
    }

    if (!size.isValid()) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative size in set_source");
        return;
    }

    auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
    surfacePrivate->pending.sourceGeometry = sourceGeometry;
}

void QWaylandWlScalerPrivate::Viewport::viewport_set_destination(QtWaylandServer::wl_viewport::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        qCDebug(qLcWaylandCompositor) << "set_destination requested for destroyed surface";
        return;
    }

    QSize destinationSize(width, height);
    if (!destinationSize.isValid() && destinationSize != QSize(-1, -1)) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative size in set_destination");
        return;
    }
    auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
    surfacePrivate->pending.destinationSize = destinationSize;
}
#endif // QT_DEPRECATED_SINCE

QT_END_NAMESPACE
