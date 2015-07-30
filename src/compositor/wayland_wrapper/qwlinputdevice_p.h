/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WLINPUTDEVICE_H
#define WLINPUTDEVICE_H

#include <stdint.h>

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandinput.h>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QScopedPointer>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif

#include <QtCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QKeyEvent;
class QTouchEvent;
class QWaylandInputDevice;
class QWaylandDrag;
class QWaylandSurfaceView;

namespace QtWayland {

class Compositor;
class DataDevice;
class Surface;
class DataDeviceManager;
class Pointer;
class Keyboard;
class Touch;
class InputMethod;

class Q_COMPOSITOR_EXPORT InputDevice : public QtWaylandServer::wl_seat
{
public:
    InputDevice(QWaylandInputDevice *handle, Compositor *compositor, QWaylandInputDevice::CapabilityFlags caps);
    ~InputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);
    void sendMouseEnterEvent(QWaylandSurfaceView *view, const QPointF &localPos);
    void sendMouseLeaveEvent(QWaylandSurfaceView *view);

    void sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullKeyEvent(QKeyEvent *event);
    void sendFullKeyEvent(Surface *surface, QKeyEvent *event);

    void sendFullTouchEvent(QTouchEvent *event);

    Surface *keyboardFocus() const;
    bool setKeyboardFocus(Surface *surface);

    QWaylandSurfaceView *mouseFocus() const;
    void setMouseFocus(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos);

    void clientRequestedDataDevice(DataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    const DataDevice *dataDevice() const;

    QWaylandOutputSpace *outputSpace() const;
    void setOutputSpace(QWaylandOutputSpace *outputSpace);

    Compositor *compositor() const;
    QWaylandInputDevice *handle() const;
    QWaylandDrag *dragHandle() const;

    Pointer *pointerDevice();
    Keyboard *keyboardDevice();
    Touch *touchDevice();
    InputMethod *inputMethod();

    const Pointer *pointerDevice() const;
    const Keyboard *keyboardDevice() const;
    const Touch *touchDevice() const;

    static InputDevice *fromSeatResource(struct ::wl_resource *resource)
    {
        return static_cast<InputDevice *>(wl_seat::Resource::fromResource(resource)->seat_object);
    }

    QWaylandInputDevice::CapabilityFlags capabilities() { return m_capabilities; }
    void setCapabilities(QWaylandInputDevice::CapabilityFlags caps);

private:
    QWaylandInputDevice *m_handle;
    QScopedPointer<QWaylandDrag> m_dragHandle;
    Compositor *m_compositor;
    QWaylandOutputSpace *m_outputSpace;
    QWaylandInputDevice::CapabilityFlags m_capabilities;

    QScopedPointer<Pointer> m_pointer;
    QScopedPointer<Keyboard> m_keyboard;
    QScopedPointer<Touch> m_touch;
    QScopedPointer<InputMethod> m_inputMethod;
    QScopedPointer<DataDevice> m_data_device;

    void seat_bind_resource(wl_seat::Resource *resource) Q_DECL_OVERRIDE;

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
