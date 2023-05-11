// Copyright (C) 2023 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "cursorshapev1.h"

namespace MockCompositor {

CursorShapeManager::CursorShapeManager(CoreCompositor *compositor, int version)
    : QtWaylandServer::wp_cursor_shape_manager_v1(compositor->m_display, version)
{
}

void CursorShapeManager::wp_cursor_shape_manager_v1_get_pointer(Resource *resource, uint32_t id, wl_resource *pointer)
{
    auto *p = fromResource<Pointer>(pointer);
    auto *cursorShape = new CursorShapeDevice(p, resource->client(), id, resource->version());
    connect(cursorShape, &QObject::destroyed, this, [this, cursorShape]() {
        m_cursorDevices.removeOne(cursorShape);
    });
    m_cursorDevices << cursorShape;
}

CursorShapeDevice::CursorShapeDevice(Pointer *pointer, wl_client *client, int id, int version)
    : QtWaylandServer::wp_cursor_shape_device_v1(client, id, version)
    , m_pointer(pointer)
{
}

void CursorShapeDevice::wp_cursor_shape_device_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void CursorShapeDevice::wp_cursor_shape_device_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void CursorShapeDevice::wp_cursor_shape_device_v1_set_shape(Resource *resource, uint32_t serial, uint32_t shape)
{
    Q_UNUSED(resource);
    m_currentShape = static_cast<CursorShapeDevice::shape>(shape);
    emit setCursor(serial);
}

}
