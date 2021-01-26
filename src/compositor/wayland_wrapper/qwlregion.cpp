/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

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
