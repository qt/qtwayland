/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
** Copyright (C) 2021 The Qt Company Ltd.
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
 * \sa {Qt Wayland Compositor Examples - Custom Shell}
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
