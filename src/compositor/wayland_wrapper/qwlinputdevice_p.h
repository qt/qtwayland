/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WLINPUTDEVICE_H
#define WLINPUTDEVICE_H

#include <stdint.h>

#include <QtCore/QList>
#include <QtCore/QPoint>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif

#include <qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QKeyEvent;
class QTouchEvent;
class QWaylandInputDevice;

namespace QtWayland {

class Compositor;
class DataDevice;
class Surface;
class DataDeviceManager;

class InputDevice : public QtWaylandServer::wl_seat, public QtWaylandServer::wl_pointer, public QtWaylandServer::wl_keyboard, public QtWaylandServer::wl_touch
{
public:
    InputDevice(QWaylandInputDevice *handle, Compositor *compositor);
    ~InputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(Surface *surface, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullKeyEvent(QKeyEvent *event);
    void sendFullTouchEvent(QTouchEvent *event);

    Surface *keyboardFocus() const;
    bool setKeyboardFocus(Surface *surface);

    Surface *mouseFocus() const;
    void setMouseFocus(Surface *surface, const QPointF &localPos, const QPointF &globalPos);

    void clientRequestedDataDevice(DataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    DataDevice *dataDevice(struct wl_client *client) const;
    void sendSelectionFocus(Surface *surface);

    Compositor *compositor() const;
    QWaylandInputDevice *handle() const;

    ::wl_pointer *pointerDevice();
    ::wl_keyboard *keyboardDevice();
    ::wl_touch *touchDevice();

    const ::wl_pointer *pointerDevice() const;
    const ::wl_keyboard *keyboardDevice() const;
    const ::wl_touch *touchDevice() const;

    static InputDevice *fromSeatResource(struct ::wl_resource *resource)
    {
        return static_cast<InputDevice *>(wl_seat::Resource::fromResource(resource)->seat);
    }

private:
    void initDevices();
    void releaseDevices();
    void cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev);
    void updateModifierState(uint key, int state);
    void sendKeyModifiers(wl_resource *resource);

    QWaylandInputDevice *m_handle;
    Compositor *m_compositor;
    QList<DataDevice *> m_data_devices;

    struct {
        ::wl_pointer pointer;
        ::wl_keyboard keyboard;
        ::wl_touch touch;
    } m_device_interfaces;

    ::wl_seat m_seat;

#ifndef QT_NO_WAYLAND_XKB
    struct xkb_keymap *m_keymap;
    struct xkb_state *m_state;
    int m_keymap_fd;
    size_t m_keymap_size;
    char *m_keymap_area;
#endif

    uint32_t toWaylandButton(Qt::MouseButton button);

    void seat_bind_resource(wl_seat::Resource *resource) Q_DECL_OVERRIDE;

    void pointer_set_cursor(wl_pointer::Resource *resource,
                            uint32_t serial,
                            struct wl_resource *surface,
                            int32_t hotspot_x,
                            int32_t hotspot_y) Q_DECL_OVERRIDE;

    void keyboard_bind_resource(wl_keyboard::Resource *resource) Q_DECL_OVERRIDE;

    void seat_get_pointer(wl_seat::Resource *resource,
                          uint32_t id) Q_DECL_OVERRIDE;
    void seat_get_keyboard(wl_seat::Resource *resource,
                           uint32_t id) Q_DECL_OVERRIDE;
    void seat_get_touch(wl_seat::Resource *resource,
                        uint32_t id) Q_DECL_OVERRIDE;

    void seat_destroy_resource(wl_seat::Resource *resource) Q_DECL_OVERRIDE;
};

}

QT_END_NAMESPACE

#endif // WLINPUTDEVICE_H
