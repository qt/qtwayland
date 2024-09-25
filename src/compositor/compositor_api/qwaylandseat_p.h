// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSEAT_P_H
#define QWAYLANDSEAT_P_H

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

#include <stdint.h>

#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QtWaylandCompositor/qwaylandseat.h>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QScopedPointer>
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QKeyEvent;
class QTouchEvent;
class QWaylandSeat;
class QWaylandDrag;
class QWaylandView;

namespace QtWayland {

class Compositor;
class DataDevice;
class Surface;
class DataDeviceManager;
class Pointer;
class Keyboard;
class Touch;
class InputMethod;

}

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandSeatPrivate : public QObjectPrivate, public QtWaylandServer::wl_seat
{
public:
    Q_DECLARE_PUBLIC(QWaylandSeat)

    QWaylandSeatPrivate(QWaylandSeat *seat);
    ~QWaylandSeatPrivate() override;

    void setCapabilities(QWaylandSeat::CapabilityFlags caps);

    static QWaylandSeatPrivate *get(QWaylandSeat *device) { return device->d_func(); }

#if QT_CONFIG(wayland_datadevice)
    void clientRequestedDataDevice(QtWayland::DataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    QtWayland::DataDevice *dataDevice() const { return data_device.data(); }
#endif

protected:
    void seat_bind_resource(wl_seat::Resource *resource) override;

    void seat_get_pointer(wl_seat::Resource *resource,
                          uint32_t id) override;
    void seat_get_keyboard(wl_seat::Resource *resource,
                           uint32_t id) override;
    void seat_get_touch(wl_seat::Resource *resource,
                        uint32_t id) override;

    void seat_destroy_resource(wl_seat::Resource *resource) override;

private:
    bool isInitialized = false;
    QWaylandCompositor *compositor = nullptr;
    QWaylandView *mouseFocus = nullptr;
    QWaylandSurface *keyboardFocus = nullptr;
    QWaylandSeat::CapabilityFlags capabilities;

    QScopedPointer<QWaylandPointer> pointer;
    QScopedPointer<QWaylandKeyboard> keyboard;
    QScopedPointer<QWaylandTouch> touch;
#if QT_CONFIG(wayland_datadevice)
    QScopedPointer<QtWayland::DataDevice> data_device;
# if QT_CONFIG(draganddrop)
    QScopedPointer<QWaylandDrag> drag_handle;
# endif
#endif
    QScopedPointer<QWaylandKeymap> keymap;
    int name = 0;
    static int max_name;
};

QT_END_NAMESPACE

#endif // QWAYLANDSEAT_P_H
