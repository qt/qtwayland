// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
    qDebug() << "count is " << m_iviSurfaces.size();
}

IviSurface::IviSurface(IviApplication *iviApplication, Surface *surface, uint32_t ivi_id, wl_client *client, int id, int version)
    : QtWaylandServer::ivi_surface(client, id, version)
    , m_iviId(ivi_id)
    , m_iviApplication(iviApplication)
    , m_surface(surface)
{
    surface->map();
}

void IviSurface::ivi_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    bool removed = m_iviApplication->m_iviSurfaces.removeOne(this);
    Q_ASSERT(removed);
    qDebug() << "destroy";

    delete this;
}

void IviSurface::ivi_surface_destroy(QtWaylandServer::ivi_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

} // namespace MockCompositor
