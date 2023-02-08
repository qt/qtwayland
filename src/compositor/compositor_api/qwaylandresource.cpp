// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandresource.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QWaylandResource
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief QWaylandResource is a container for a \c wl_resource.
 *
 * The QWaylandResource is a simple wrapper around the Wayland type \c wl_resource, and makes it
 * possible to use wl_resource pointers in Qt Quick APIs.
 *
 * \sa {Custom Shell}
 */

/*!
 * Constructs an invalid QWaylandResource. The \l{resource()} accessor will return null.
 */
QWaylandResource::QWaylandResource()
{
}

/*!
 * Constructs a QWaylandResource which contains \a resource.
 */
QWaylandResource::QWaylandResource(wl_resource *resource)
                : m_resource(resource)
{
}

/*!
 * \fn wl_resource *QWaylandResource::resource() const
 *
 * \return the wl_resource pointer held by this QWaylandResource.
 */

QT_END_NAMESPACE

#include "moc_qwaylandresource.cpp"
