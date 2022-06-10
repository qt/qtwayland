// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WLDATADEVICE_H
#define WLDATADEVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QtWaylandCompositor/QWaylandSeat>

QT_REQUIRE_CONFIG(wayland_datadevice);

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Compositor;
class DataSource;
class Seat;
class Surface;

class DataDevice : public QtWaylandServer::wl_data_device
{
public:
    DataDevice(QWaylandSeat *seat);

    void setFocus(QWaylandClient *client);
    void sourceDestroyed(DataSource *source);

#if QT_CONFIG(draganddrop)
    void setDragFocus(QWaylandSurface *focus, const QPointF &localPosition);

    QWaylandSurface *dragIcon() const;
    QWaylandSurface *dragOrigin() const;

    void dragMove(QWaylandSurface *target, const QPointF &pos);
    void drop();
    void cancelDrag();
#endif

protected:
#if QT_CONFIG(draganddrop)
    void data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial) override;
#endif
    void data_device_set_selection(Resource *resource, struct ::wl_resource *source, uint32_t serial) override;

private:
#if QT_CONFIG(draganddrop)
    void setDragIcon(QWaylandSurface *icon);
#endif

    QWaylandCompositor *m_compositor = nullptr;
    QWaylandSeat *m_seat = nullptr;

    DataSource *m_selectionSource = nullptr;

#if QT_CONFIG(draganddrop)
    struct ::wl_client *m_dragClient = nullptr;
    DataSource *m_dragDataSource = nullptr;

    QWaylandSurface *m_dragFocus = nullptr;
    Resource *m_dragFocusResource = nullptr;

    QWaylandSurface *m_dragIcon = nullptr;
    QWaylandSurface *m_dragOrigin = nullptr;
#endif
};

}

QT_END_NAMESPACE

#endif // WLDATADEVICE_H
