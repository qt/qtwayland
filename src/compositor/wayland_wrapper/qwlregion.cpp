// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlregion_p.h"

#include <QtWaylandCompositor/private/qwaylandutils_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Region::Region(struct wl_client *client, uint32_t id)
    : QtWaylandServer::wl_region(client, id, 1)
{
}

Region::~Region()
{
}

Region *Region::fromResource(struct ::wl_resource *resource)
{
    return QtWayland::fromResource<Region *>(resource);
}

void Region::region_destroy_resource(Resource *)
{
    delete this;
}

void Region::region_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void Region::region_add(Resource *, int32_t x, int32_t y, int32_t w, int32_t h)
{
    m_region += QRect(x, y, w, h);
}

void Region::region_subtract(Resource *, int32_t x, int32_t y, int32_t w, int32_t h)
{
    m_region -= QRect(x, y, w, h);
}

}

QT_END_NAMESPACE
