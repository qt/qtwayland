/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandviewporter_p.h"

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QWaylandViewporter
    \inmodule QtWaylandCompositor
    \since 5.13
    \brief Provides an extension for surface resizing and cropping.

    The QWaylandViewporter extension provides a way for clients to resize and crop surface
    contents.

    QWaylandViewporter corresponds to the Wayland interface, \c wp_viewporter.
*/

/*!
    Constructs a QWaylandViewporter object.
*/
QWaylandViewporter::QWaylandViewporter()
    : QWaylandCompositorExtensionTemplate<QWaylandViewporter>(*new QWaylandViewporterPrivate)
{
}

/*!
 * Constructs a QWaylandViewporter object for the provided \a compositor.
 */
QWaylandViewporter::QWaylandViewporter(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandViewporter>(compositor, *new QWaylandViewporterPrivate())
{
}

/*!
    Initializes the extension.
*/
void QWaylandViewporter::initialize()
{
    Q_D(QWaylandViewporter);

    QWaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandViewporter";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
    Returns the Wayland interface for the QWaylandViewporter.
*/
const wl_interface *QWaylandViewporter::interface()
{
    return QWaylandViewporterPrivate::interface();
}

void QWaylandViewporterPrivate::wp_viewporter_destroy(Resource *resource)
{
    // Viewport objects are allowed ot outlive the viewporter
    wl_resource_destroy(resource->handle);
}

void QWaylandViewporterPrivate::wp_viewporter_get_viewport(Resource *resource, uint id, wl_resource *surfaceResource)
{
    auto *surface = QWaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qWarning() << "Couldn't find surface for viewporter";
        return;
    }

    auto *surfacePrivate = QWaylandSurfacePrivate::get(surface);
    if (surfacePrivate->viewport) {
        wl_resource_post_error(resource->handle, WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS,
                               "viewport already exists for surface");
        return;
    }

    surfacePrivate->viewport = new Viewport(surface, resource->client(), id);
}

QWaylandViewporterPrivate::Viewport::Viewport(QWaylandSurface *surface, wl_client *client, int id)
    : QtWaylandServer::wp_viewport(client, id, /*version*/ 1)
    , m_surface(surface)
{
    Q_ASSERT(surface);
}

QWaylandViewporterPrivate::Viewport::~Viewport()
{
    if (m_surface) {
        auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
        Q_ASSERT(surfacePrivate->viewport == this);
        surfacePrivate->viewport = nullptr;
    }
}

// This function has to be called immediately after a surface is committed, before no
// other client events have been dispatched, or we may incorrectly error out on an
// incomplete pending state. See comment below.
void QWaylandViewporterPrivate::Viewport::checkCommittedState()
{
    auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);

    // We can't use the current state for destination/source when checking,
    // as that has fallbacks to the buffer size so we can't distinguish
    // between the set/unset case. We use the pending state because no other
    // requests has modified it yet.
    QSize destination = surfacePrivate->pending.destinationSize;
    QRectF source = surfacePrivate->pending.sourceGeometry;

    if (!destination.isValid() && source.size() != source.size().toSize()) {
        wl_resource_post_error(resource()->handle, error_bad_size,
                               "non-integer size (%fx%f) with unset destination",
                               source.width(), source.height());
        return;
    }

    QRectF max = QRectF(QPointF(), m_surface->bufferSize() / m_surface->bufferScale());
    // We can't use QRectF.contains, because that would return false for values on the border
    if (max.united(source) != max) {
        wl_resource_post_error(resource()->handle, error_out_of_buffer,
                               "source %f,%f, %fx%f extends outside attached buffer %fx%f",
                               source.x(), source.y(), source.width(), source.height(),
                               max.width(), max.height());
        return;
    }
}


void QWaylandViewporterPrivate::Viewport::wp_viewport_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandViewporterPrivate::Viewport::wp_viewport_destroy(Resource *resource)
{
    if (m_surface) {
        auto *surfacePrivate = QWaylandSurfacePrivate::get(m_surface);
        surfacePrivate->pending.destinationSize = QSize();
        surfacePrivate->pending.sourceGeometry = QRectF();
    }
    wl_resource_destroy(resource->handle);
}

void QWaylandViewporterPrivate::Viewport::wp_viewport_set_source(QtWaylandServer::wp_viewport::Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        wl_resource_post_error(resource->handle, error_no_surface,
                               "set_source requested for destroyed surface");
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

void QWaylandViewporterPrivate::Viewport::wp_viewport_set_destination(QtWaylandServer::wp_viewport::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        wl_resource_post_error(resource->handle, error_no_surface,
                               "set_destination requested for destroyed surface");
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

QT_END_NAMESPACE
