/****************************************************************************
**
** Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "iviapplication.h"

namespace MockCompositor {

IviApplication::IviApplication(CoreCompositor *compositor)
{
    init(compositor->m_display, 1);
}

void IviApplication::ivi_application_surface_create(Resource *resource, uint32_t ivi_id, struct ::wl_resource *surface, uint32_t id)
{
    auto *s = fromResource<Surface>(surface);
    auto *iviSurface = new IviSurface(this, s, ivi_id, resource->client(), id, resource->version());
    m_iviSurfaces << iviSurface;
    qDebug() << "count is " << m_iviSurfaces.count();
}

IviSurface::IviSurface(IviApplication *iviApplication, Surface *surface, uint32_t ivi_id, wl_client *client, int id, int version)
    : QtWaylandServer::ivi_surface(client, id, version)
    , m_iviId(ivi_id)
    , m_iviApplication(iviApplication)
{
    Q_UNUSED(surface);
}

void IviSurface::ivi_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    bool removed = m_iviApplication->m_iviSurfaces.removeOne(this);
    Q_ASSERT(removed);
        qDebug() << "destriy";

    delete this;
}

void IviSurface::ivi_surface_destroy(QtWaylandServer::ivi_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

} // namespace MockCompositor
