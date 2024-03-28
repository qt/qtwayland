// Copyright (C) 2023 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKCOMPOSITOR_CURSORSHAPE_H
#define MOCKCOMPOSITOR_CURSORSHAPE_H

#include "coreprotocol.h"
#include <qwayland-server-cursor-shape-v1.h>

namespace MockCompositor {

class CursorShapeDevice;

class CursorShapeManager : public Global, public QtWaylandServer::wp_cursor_shape_manager_v1
{
    Q_OBJECT
public:
    explicit CursorShapeManager(CoreCompositor *compositor, int version = 1);
    QList<CursorShapeDevice *> m_cursorDevices;

protected:
    void wp_cursor_shape_manager_v1_get_pointer(Resource *resource, uint32_t id, wl_resource *pointer) override;
};

class CursorShapeDevice : public QObject, public QtWaylandServer::wp_cursor_shape_device_v1
{
    Q_OBJECT
public:
    explicit CursorShapeDevice(Pointer *pointer, wl_client *client, int id, int version);
    Pointer *m_pointer;
    shape m_currentShape = shape_default;

Q_SIGNALS:
    void setCursor(uint serial);

protected:
    void wp_cursor_shape_device_v1_destroy_resource(Resource *resource) override;
    void wp_cursor_shape_device_v1_destroy(Resource *resource) override;
    void wp_cursor_shape_device_v1_set_shape(Resource *resource, uint32_t serial, uint32_t shape) override;
};

}

#endif
