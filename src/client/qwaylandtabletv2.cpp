/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandtabletv2_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandsurface_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandTabletManagerV2::QWaylandTabletManagerV2(QWaylandDisplay *display, uint id, uint version)
    : zwp_tablet_manager_v2(display->wl_registry(), id, qMin(version, uint(1)))
{
    // Create tabletSeats for all seats.
    // This only works if we get the manager after all seats
    const auto seats = display->inputDevices();
    for (auto *seat : seats)
        createTabletSeat(seat);
}

QWaylandTabletSeatV2 *QWaylandTabletManagerV2::createTabletSeat(QWaylandInputDevice *seat)
{
    return new QWaylandTabletSeatV2(this, seat);
}

QWaylandTabletSeatV2::QWaylandTabletSeatV2(QWaylandTabletManagerV2 *manager, QWaylandInputDevice *seat)
    : QtWayland::zwp_tablet_seat_v2(manager->get_tablet_seat(seat->wl_seat()))
{
}

QWaylandTabletSeatV2::~QWaylandTabletSeatV2()
{
    for (auto *tablet : m_tablets)
        tablet->destroy();
    for (auto *tool : m_tools)
        tool->destroy();
    for (auto *pad : m_pads)
        pad->destroy();
    destroy();
}

void QWaylandTabletSeatV2::zwp_tablet_seat_v2_tablet_added(zwp_tablet_v2 *id)
{
    auto *tablet = new QWaylandTabletV2(id);
    m_tablets.push_back(tablet);
    connect(tablet, &QWaylandTabletV2::destroyed, this, [this, tablet] { m_tablets.removeOne(tablet); });
}

void QWaylandTabletSeatV2::zwp_tablet_seat_v2_tool_added(zwp_tablet_tool_v2 *id)
{
    auto *tool = new QWaylandTabletToolV2(id);
    m_tools.push_back(tool);
    connect(tool, &QWaylandTabletToolV2::destroyed, this, [this, tool] { m_tools.removeOne(tool); });
}

void QWaylandTabletSeatV2::zwp_tablet_seat_v2_pad_added(zwp_tablet_pad_v2 *id)
{
    auto *pad = new QWaylandTabletPadV2(id);
    m_pads.push_back(pad);
    connect(pad, &QWaylandTabletPadV2::destroyed, this, [this, pad] { m_pads.removeOne(pad); });
}

QWaylandTabletV2::QWaylandTabletV2(::zwp_tablet_v2 *tablet)
    : QtWayland::zwp_tablet_v2(tablet)
{
}

void QWaylandTabletV2::zwp_tablet_v2_removed()
{
    destroy();
    delete this;
}

QWaylandTabletToolV2::QWaylandTabletToolV2(::zwp_tablet_tool_v2 *tool)
    : QtWayland::zwp_tablet_tool_v2(tool)
{
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_type(uint32_t tool_type)
{
    m_toolType = type(tool_type);
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_hardware_serial(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo)
{
    m_uid = (quint64(hardware_serial_hi) << 32) + hardware_serial_lo;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_capability(uint32_t capability)
{
    if (capability == capability_rotation)
        m_hasRotation = true;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_done()
{
    switch (m_toolType) {
    case type::type_airbrush:
    case type::type_brush:
    case type::type_pencil:
    case type::type_pen:
        m_pointerType = QTabletEvent::PointerType::Pen;
        break;
    case type::type_eraser:
        m_pointerType = QTabletEvent::PointerType::Eraser;
        break;
    case type::type_mouse:
    case type::type_lens:
        m_pointerType = QTabletEvent::PointerType::Cursor;
        break;
    case type::type_finger:
        m_pointerType = QTabletEvent::PointerType::UnknownPointer;
        break;
    }
    switch (m_toolType) {
    case type::type_airbrush:
        m_tabletDevice = QTabletEvent::TabletDevice::Airbrush;
        break;
    case type::type_brush:
    case type::type_pencil:
    case type::type_pen:
    case type::type_eraser:
        m_tabletDevice = m_hasRotation ? QTabletEvent::TabletDevice::RotationStylus : QTabletEvent::TabletDevice::Stylus;
        break;
    case type::type_lens:
        m_tabletDevice = QTabletEvent::TabletDevice::Puck;
        break;
    case type::type_mouse:
    case type::type_finger:
        m_tabletDevice = QTabletEvent::TabletDevice::NoDevice;
        break;
    }
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_removed()
{
    destroy();
    delete this;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_proximity_in(uint32_t serial, zwp_tablet_v2 *tablet, wl_surface *surface)
{
    Q_UNUSED(tablet);
    Q_UNUSED(serial);
    if (Q_UNLIKELY(!surface)) {
        qCDebug(lcQpaWayland) << "Ignoring zwp_tablet_tool_v2_proximity_v2 with no surface";
        return;
    }
    m_pending.enteredSurface = true;
    m_pending.proximitySurface = QWaylandSurface::fromWlSurface(surface);
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_proximity_out()
{
    m_pending.enteredSurface = false;
    m_pending.proximitySurface = nullptr;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_down(uint32_t serial)
{
    Q_UNUSED(serial);
    m_pending.down = true;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_up()
{
    m_pending.down = false;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_motion(wl_fixed_t x, wl_fixed_t y)
{
    m_pending.surfacePosition = QPointF(wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_pressure(uint32_t pressure)
{
    const int maxPressure = 65535;
    m_pending.pressure = qreal(pressure)/maxPressure;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_distance(uint32_t distance)
{
    m_pending.distance = distance;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_tilt(wl_fixed_t tilt_x, wl_fixed_t tilt_y)
{
    m_pending.xTilt = wl_fixed_to_double(tilt_x);
    m_pending.yTilt = wl_fixed_to_double(tilt_y);
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_rotation(wl_fixed_t degrees)
{
    m_pending.rotation = wl_fixed_to_double(degrees);
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_slider(int32_t position)
{
    m_pending.slider = qreal(position) / 65535;
}

static Qt::MouseButton mouseButtonFromTablet(uint button)
{
    switch (button) {
    case 0x110: return Qt::MouseButton::LeftButton; // BTN_LEFT
    case 0x14b: return Qt::MouseButton::MiddleButton; // BTN_STYLUS
    case 0x14c: return Qt::MouseButton::RightButton; // BTN_STYLUS2
    default:
        return Qt::NoButton;
    }
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_button(uint32_t serial, uint32_t button, uint32_t state)
{
    Q_UNUSED(serial);
    Qt::MouseButton mouseButton = mouseButtonFromTablet(button);
    if (state == button_state_pressed)
        m_pending.buttons |= mouseButton;
    else
        m_pending.buttons &= ~mouseButton;
}

void QWaylandTabletToolV2::zwp_tablet_tool_v2_frame(uint32_t time)
{
    if (m_pending.proximitySurface && !m_applied.proximitySurface) {
        QWindowSystemInterface::handleTabletEnterProximityEvent(m_tabletDevice, m_pointerType, m_uid);
        m_applied.proximitySurface = m_pending.proximitySurface;
    }

    if (!(m_pending == m_applied) && m_pending.proximitySurface) {
        if (!m_pending.proximitySurface) {
            qCWarning(lcQpaWayland) << "Can't send tablet event with no proximity surface, ignoring";
            return;
        }
        QWaylandWindow *waylandWindow = QWaylandWindow::fromWlSurface(m_pending.proximitySurface->object());
        QWindow *window = waylandWindow->window();
        ulong timestamp = time;
        const QPointF localPosition = waylandWindow->mapFromWlSurface(m_pending.surfacePosition);

        QPointF delta = localPosition - localPosition.toPoint();
        QPointF globalPosition = window->mapToGlobal(localPosition.toPoint());
        globalPosition += delta;

        Qt::MouseButtons buttons = m_pending.down ? Qt::MouseButton::LeftButton : Qt::MouseButton::NoButton;
        buttons |= m_pending.buttons;
        qreal pressure = m_pending.pressure;
        int xTilt = int(m_pending.xTilt);
        int yTilt = int(m_pending.yTilt);
        qreal tangentialPressure = m_pending.slider;
        qreal rotation = m_pending.rotation;
        int z = int(m_pending.distance);
        QWindowSystemInterface::handleTabletEvent(window, timestamp, localPosition, globalPosition,
                                                  m_tabletDevice, m_pointerType, buttons, pressure,
                                                  xTilt, yTilt, tangentialPressure, rotation, z, m_uid);
    }

    if (!m_pending.proximitySurface && m_applied.enteredSurface) {
        QWindowSystemInterface::handleTabletLeaveProximityEvent(m_tabletDevice, m_pointerType, m_uid);
        m_pending = State(); // Don't leave pressure etc. lying around when we enter the next surface
    }

    m_applied = m_pending;
}

// TODO: delete when upgrading to c++20
bool QWaylandTabletToolV2::State::operator==(const QWaylandTabletToolV2::State &o) const {
    return
            down == o.down &&
            proximitySurface.data() == o.proximitySurface.data() &&
            enteredSurface == o.enteredSurface &&
            surfacePosition == o.surfacePosition &&
            distance == o.distance &&
            pressure == o.pressure &&
            rotation == o.rotation &&
            xTilt == o.xTilt &&
            yTilt == o.yTilt &&
            slider == o.slider &&
            buttons == o.buttons;
}

QWaylandTabletPadV2::QWaylandTabletPadV2(::zwp_tablet_pad_v2 *pad)
    : QtWayland::zwp_tablet_pad_v2(pad)
{
}

void QWaylandTabletPadV2::zwp_tablet_pad_v2_removed()
{
    destroy();
    delete this;
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
