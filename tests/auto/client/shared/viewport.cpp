// Copyright (C) 2022 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "viewport.h"

namespace MockCompositor {

Viewporter::Viewporter(CoreCompositor *compositor, int version)
    : QtWaylandServer::wp_viewporter(compositor->m_display, version)
{
}

void Viewporter::wp_viewporter_get_viewport(Resource *resource, uint32_t id, wl_resource *surface)
{
    auto *s = fromResource<Surface>(surface);
    auto *viewport = new Viewport(s, resource->client(), id, resource->version());
    connect(viewport, &QObject::destroyed, this, [this, viewport]() {
        m_viewports.removeOne(viewport);
    });
    m_viewports << viewport;
}

Viewport::Viewport(Surface *surface, wl_client *client, int id, int version)
    : QtWaylandServer::wp_viewport(client, id, version)
    , m_surface(surface)
{
}

void Viewport::wp_viewport_set_source(Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
    Q_UNUSED(resource)
    m_source = QRectF(wl_fixed_to_double(x),
                      wl_fixed_to_double(y),
                      wl_fixed_to_double(width),
                      wl_fixed_to_double(height));
    Q_EMIT sourceChanged();
}

void Viewport::wp_viewport_set_destination(Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource)

    m_destination = QSize(width, height);
    Q_EMIT destinationChanged();
}

void Viewport::wp_viewport_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void Viewport::wp_viewport_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

}
