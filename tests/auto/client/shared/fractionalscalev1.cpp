// Copyright (C) 2022 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "fractionalscalev1.h"

namespace MockCompositor {

FractionalScaleManager::FractionalScaleManager(CoreCompositor *compositor, int version)
    : QtWaylandServer::wp_fractional_scale_manager_v1(compositor->m_display, version)
{
}

void FractionalScaleManager::wp_fractional_scale_manager_v1_get_fractional_scale(Resource *resource, uint32_t id, wl_resource *surface)
{
    auto *s = fromResource<Surface>(surface);
    auto *scaler = new FractionalScale(s, resource->client(), id, resource->version());
    connect(scaler, &QObject::destroyed, this, [this, scaler]() {
        m_fractionalScales.removeOne(scaler);
    });
    m_fractionalScales << scaler;
}

FractionalScale::FractionalScale(Surface *surface, wl_client *client, int id, int version)
    : QtWaylandServer::wp_fractional_scale_v1(client, id, version)
    , m_surface(surface)
{
}

void FractionalScale::wp_fractional_scale_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void FractionalScale::wp_fractional_scale_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

}
