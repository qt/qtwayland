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

#include "qwlinputdevice_p.h"

#include "qwlcompositor_p.h"
#include "qwldatadevice_p.h"
#include "qwlsurface_p.h"
#include "qwltouch_p.h"
#include "qwlqtkey_p.h"
#include "qwaylandcompositor.h"

#include <QtGui/QTouchEvent>

#ifndef QT_NO_WAYLAND_XKB
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#endif

QT_BEGIN_NAMESPACE

namespace QtWayland {

InputDevice::InputDevice(QWaylandInputDevice *handle, Compositor *compositor)
    : QtWaylandServer::wl_seat(compositor->wl_display())
    , m_handle(handle)
    , m_compositor(compositor)
{
    wl_seat_init(&m_seat);
    initDevices();

#ifndef QT_NO_WAYLAND_XKB
    xkb_rule_names xkb_names;
    xkb_context *context = xkb_context_new(xkb_context_flags(0));

    memset(&xkb_names, 0, sizeof(xkb_names));
    xkb_names.rules = strdup("evdev");
    xkb_names.model = strdup("pc105");
    xkb_names.layout = strdup("us");

    xkb_keymap *keymap = xkb_map_new_from_names(context, &xkb_names, xkb_map_compile_flags(0));
    if (!keymap)
        qFatal("Failed to compile global XKB keymap");

    char *keymap_str_data = xkb_map_get_as_string(keymap);
    QByteArray keymap_str = keymap_str_data;
    m_keymap_size = keymap_str.size() + 1;
    free(keymap_str_data);

    const char *path = getenv("XDG_RUNTIME_DIR");
    if (!path)
        qFatal("XDG_RUNTIME_DIR not set");

    QByteArray name = QByteArray(path) + "/qtwayland-xkb-map-XXXXXX";

    int fd = mkstemp(name.data());
    if (fd >= 0) {
        long flags = fcntl(fd, F_GETFD);
        if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
            close(fd);
            qFatal("Failed to set FD_CLOEXEC on anonymous file");
        }
        unlink(name.data());
    } else {
        qFatal("Failed to create anonymous file with name %s", name.constData());
    }

    if (ftruncate(fd, m_keymap_size) < 0)
        qFatal("Failed to create anonymous file of size %lu", (unsigned long)m_keymap_size);

    m_keymap_fd = fd;

    m_keymap_area = (char *)mmap(0, m_keymap_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_keymap_fd, 0);
    if (m_keymap_area == MAP_FAILED) {
        close(m_keymap_fd);
        qFatal("Failed to map shared memory segment");
    }

    strcpy(m_keymap_area, keymap_str.constData());

    m_state = xkb_state_new(keymap);

    free((char *)xkb_names.rules);
    free((char *)xkb_names.model);
    free((char *)xkb_names.layout);
    xkb_map_unref(keymap);
    xkb_context_unref(context);
#endif
}

InputDevice::~InputDevice()
{
    qDeleteAll(m_data_devices);
    releaseDevices();

#ifndef QT_NO_WAYLAND_XKB
    if (m_keymap_area)
        munmap(m_keymap_area, m_keymap_size);
    close(m_keymap_fd);
    xkb_state_unref(m_state);
#endif
}

void InputDevice::initDevices()
{
    wl_pointer_init(&m_device_interfaces.pointer);
    wl_seat_set_pointer(&m_seat, &m_device_interfaces.pointer);

    wl_keyboard_init(&m_device_interfaces.keyboard);
    wl_seat_set_keyboard(&m_seat, &m_device_interfaces.keyboard);

    wl_touch_init(&m_device_interfaces.touch);
    wl_seat_set_touch(&m_seat, &m_device_interfaces.touch);
}

void InputDevice::releaseDevices()
{
    wl_pointer_release(&m_device_interfaces.pointer);
    wl_keyboard_release(&m_device_interfaces.keyboard);
    wl_touch_release(&m_device_interfaces.touch);
}

wl_pointer *InputDevice::pointerDevice()
{
    return &m_device_interfaces.pointer;
}

wl_keyboard *InputDevice::keyboardDevice()
{
    return &m_device_interfaces.keyboard;
}

wl_touch *InputDevice::touchDevice()
{
    return &m_device_interfaces.touch;
}

const wl_pointer *InputDevice::pointerDevice() const
{
    return &m_device_interfaces.pointer;
}

const wl_keyboard *InputDevice::keyboardDevice() const
{
    return &m_device_interfaces.keyboard;
}

const wl_touch *InputDevice::touchDevice() const
{
    return &m_device_interfaces.touch;
}

void InputDevice::seat_destroy_resource(wl_seat::Resource *resource)
{
    if (keyboardDevice()->focus_resource == resource->handle)
        keyboardDevice()->focus_resource = 0;

    if (pointerDevice()->focus_resource == resource->handle)
        pointerDevice()->focus_resource = 0;

    cleanupDataDeviceForClient(resource->client(), true);
}

void InputDevice::seat_bind_resource(wl_seat::Resource *resource)
{
    wl_list_insert(&m_seat.base_resource_list, &resource->handle->link);

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD;
    if (!QTouchDevice::devices().isEmpty())
        caps |= WL_SEAT_CAPABILITY_TOUCH;

    wl_seat::send_capabilities(resource->handle, caps);
}

void InputDevice::pointer_set_cursor(wl_pointer::Resource *resource,
                                     uint32_t serial, wl_resource *surface_resource,
                                     int32_t hotspot_x, int32_t hotspot_y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    /* Hide cursor */
    if (!surface_resource)
    {
        m_compositor->waylandCompositor()->setCursorSurface(NULL, 0, 0);
        return;
    }

    QtWayland::Surface *surface = QtWayland::Surface::fromResource(surface_resource);
    surface->setCursorSurface(true);
    m_compositor->waylandCompositor()->setCursorSurface(surface->waylandSurface(), hotspot_x, hotspot_y);
}

void InputDevice::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    ::wl_pointer *pointer = pointerDevice();
    wl_pointer::add(&pointer->resource_list, resource->client(), id);
}

void InputDevice::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    ::wl_keyboard *keyboard = keyboardDevice();
    wl_keyboard::add(&keyboard->resource_list, resource->client(), id);
}

void InputDevice::keyboard_bind_resource(wl_keyboard::Resource *resource)
{
#ifndef QT_NO_WAYLAND_XKB
    wl_keyboard::send_keymap(resource->handle, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                             m_keymap_fd, m_keymap_size);
#endif
}

void InputDevice::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    ::wl_touch *touch = touchDevice();
    wl_touch::add(&touch->resource_list, resource->client(), id);
}

void InputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);
    ::wl_pointer *pointer = pointerDevice();
    pointer->button_count++;
    uint32_t time = m_compositor->currentTimeMsecs();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab, time, toWaylandButton(button), 1);
}

void InputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);
    ::wl_pointer *pointer = pointerDevice();
    pointer->button_count--;
    uint32_t time = m_compositor->currentTimeMsecs();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab, time, toWaylandButton(button), 0);
}

void InputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    Q_UNUSED(globalPos);
    uint32_t time = m_compositor->currentTimeMsecs();
    ::wl_pointer *pointer = pointerDevice();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    pointer->x = wl_fixed_from_double(globalPos.x());
    pointer->y = wl_fixed_from_double(globalPos.y());
    interface->motion(pointer->grab,
                      time,
                      wl_fixed_from_double(localPos.x()), wl_fixed_from_double(localPos.y()));
}

void InputDevice::sendMouseMoveEvent(Surface *surface, const QPointF &localPos, const QPointF &globalPos)
{
    setMouseFocus(surface,localPos,globalPos);
    sendMouseMoveEvent(localPos,globalPos);
}

void InputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    ::wl_pointer *pointer = pointerDevice();
    struct wl_resource *resource = pointer->focus_resource;
    if (!resource)
        return;
    uint32_t time = m_compositor->currentTimeMsecs();
    uint32_t axis = orientation == Qt::Horizontal ? WL_POINTER_AXIS_HORIZONTAL_SCROLL
                                                  : WL_POINTER_AXIS_VERTICAL_SCROLL;
    wl_pointer_send_axis(resource, time, axis, wl_fixed_from_int(-delta / 12));
}

void InputDevice::updateModifierState(uint code, int state)
{
#ifndef QT_NO_WAYLAND_XKB
    xkb_state_update_key(m_state, code, state ? XKB_KEY_DOWN : XKB_KEY_UP);

    uint32_t mods_depressed = xkb_state_serialize_mods(m_state, (xkb_state_component)XKB_STATE_DEPRESSED);
    uint32_t mods_latched = xkb_state_serialize_mods(m_state, (xkb_state_component)XKB_STATE_LATCHED);
    uint32_t mods_locked = xkb_state_serialize_mods(m_state, (xkb_state_component)XKB_STATE_LATCHED);
    uint32_t group = xkb_state_serialize_group(m_state, (xkb_state_component)XKB_STATE_EFFECTIVE);

    ::wl_keyboard *keyboard = keyboardDevice();

    if (mods_depressed == keyboard->modifiers.mods_depressed
        && mods_latched == keyboard->modifiers.mods_latched
        && mods_locked == keyboard->modifiers.mods_locked
        && group == keyboard->modifiers.group)
    {
        return; // no change
    }

    keyboard->modifiers.mods_depressed = mods_depressed;
    keyboard->modifiers.mods_latched = mods_latched;
    keyboard->modifiers.mods_locked = mods_locked;
    keyboard->modifiers.group = group;

    if (keyboard->focus_resource)
        sendKeyModifiers(keyboard->focus_resource);
#else
    Q_UNUSED(code);
    Q_UNUSED(state);
#endif
}

void InputDevice::sendKeyModifiers(wl_resource *resource)
{
    ::wl_keyboard *keyboard = keyboardDevice();
    uint32_t serial = wl_display_next_serial(m_compositor->wl_display());
    wl_keyboard_send_modifiers(resource, serial, keyboard->modifiers.mods_depressed,
        keyboard->modifiers.mods_latched, keyboard->modifiers.mods_locked, keyboard->modifiers.group);
}

void InputDevice::sendKeyPressEvent(uint code)
{
    ::wl_keyboard *keyboard = keyboardDevice();
    if (keyboard->focus_resource) {
        uint32_t time = m_compositor->currentTimeMsecs();
        uint32_t serial = wl_display_next_serial(m_compositor->wl_display());
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code - 8, 1);
    }
    updateModifierState(code, 1);
}

void InputDevice::sendKeyReleaseEvent(uint code)
{
    ::wl_keyboard *keyboard = keyboardDevice();
    if (keyboard->focus_resource) {
        uint32_t time = m_compositor->currentTimeMsecs();
        uint32_t serial = wl_display_next_serial(m_compositor->wl_display());
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code - 8, 0);
    }
    updateModifierState(code, 0);
}

void InputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    uint32_t time = m_compositor->currentTimeMsecs();
    uint32_t serial = 0;
    ::wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (!resource)
        return;
    switch (state) {
    case Qt::TouchPointPressed:
        wl_touch_send_down(resource, serial, time, &touch->focus->resource, id,
                           wl_fixed_from_double(x), wl_fixed_from_double(y));
        break;
    case Qt::TouchPointMoved:
        wl_touch_send_motion(resource, time, id,
                             wl_fixed_from_double(x), wl_fixed_from_double(y));
        break;
    case Qt::TouchPointReleased:
        wl_touch_send_up(resource, serial, time, id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

void InputDevice::sendTouchFrameEvent()
{
    ::wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource)
        wl_touch_send_frame(resource);
}

void InputDevice::sendTouchCancelEvent()
{
    ::wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource)
        wl_touch_send_cancel(resource);
}

void InputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    QtKeyExtensionGlobal *ext = m_compositor->qtkeyExtension();
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (event->type() == QEvent::KeyPress)
        sendKeyPressEvent(event->nativeScanCode());
    else if (event->type() == QEvent::KeyRelease)
        sendKeyReleaseEvent(event->nativeScanCode());
}

void InputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    if (!mouseFocus()) {
        qWarning("Cannot send touch event, no pointer focus, fix the compositor");
        return;
    }

    if (event->type() == QEvent::TouchCancel) {
        sendTouchCancelEvent();
        return;
    }

    TouchExtensionGlobal *ext = m_compositor->touchExtension();
    if (ext && ext->postTouchEvent(event, mouseFocus()))
        return;

    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    QPointF pos = mouseFocus()->pos();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        QPointF p = tp.pos() - pos;
        sendTouchPointEvent(tp.id(), p.x(), p.y(), tp.state());
    }
    sendTouchFrameEvent();
}

Surface *InputDevice::keyboardFocus() const
{
    return static_cast<Surface *>(keyboardDevice()->focus);
}

/*!
 * \return True if the keyboard focus is changed successfully. False for inactive transient surfaces.
 */
bool InputDevice::setKeyboardFocus(Surface *surface)
{
    if (surface && surface->transientInactive())
        return false;

    sendSelectionFocus(surface);
    wl_keyboard_set_focus(keyboardDevice(), surface);
    return true;
}

Surface *InputDevice::mouseFocus() const
{
    return static_cast<Surface *>(pointerDevice()->focus);
}

void InputDevice::setMouseFocus(Surface *surface, const QPointF &localPos, const QPointF &globalPos)
{
    ::wl_pointer *pointer = pointerDevice();
    pointer->x = wl_fixed_from_double(globalPos.x());
    pointer->y = wl_fixed_from_double(globalPos.y());
    pointer->current = surface;
    pointer->current_x = wl_fixed_from_double(localPos.x());
    pointer->current_y = wl_fixed_from_double(localPos.y());
    pointer->grab->interface->focus(pointer->grab, surface,
                                    wl_fixed_from_double(localPos.x()), wl_fixed_from_double(localPos.y()));

    // We have no separate touch focus management so make it match the pointer focus always.
    // No wl_touch_set_focus() is available so set it manually.
    ::wl_touch *touch = touchDevice();
    touch->focus = surface;
    touch->focus_resource = Compositor::resourceForSurface(&touch->resource_list, surface);
}

void InputDevice::cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev)
{
    for (int i = 0; i < m_data_devices.size(); i++) {
        struct wl_resource *data_device_resource =
                m_data_devices.at(i)->dataDeviceResource();
        if (data_device_resource->client == client) {
            if (destroyDev)
                delete m_data_devices.at(i);
            m_data_devices.removeAt(i);
            break;
        }
    }
}

void InputDevice::clientRequestedDataDevice(DataDeviceManager *data_device_manager, struct wl_client *client, uint32_t id)
{
    cleanupDataDeviceForClient(client, false);
    DataDevice *dataDevice = new DataDevice(data_device_manager,client,id);
    m_data_devices.append(dataDevice);
}

void InputDevice::sendSelectionFocus(Surface *surface)
{
    if (!surface)
        return;
    DataDevice *device = dataDevice(surface->resource()->client());
    if (device) {
        device->sendSelectionFocus();
    }
}

Compositor *InputDevice::compositor() const
{
    return m_compositor;
}

QWaylandInputDevice *InputDevice::handle() const
{
    return m_handle;
}

uint32_t InputDevice::toWaylandButton(Qt::MouseButton button)
{
#ifndef BTN_LEFT
    uint32_t BTN_LEFT = 0x110;
#endif
    // the range of valid buttons (evdev module) is from 0x110
    // through 0x11f. 0x120 is the first 'Joystick' button.
    switch (button) {
    case Qt::LeftButton: return BTN_LEFT;
    case Qt::RightButton: return uint32_t(0x111);
    case Qt::MiddleButton: return uint32_t(0x112);
    case Qt::ExtraButton1: return uint32_t(0x113);  // AKA Qt::BackButton, Qt::XButton1
    case Qt::ExtraButton2: return uint32_t(0x114);  // AKA Qt::ForwardButton, Qt::XButton2
    case Qt::ExtraButton3: return uint32_t(0x115);
    case Qt::ExtraButton4: return uint32_t(0x116);
    case Qt::ExtraButton5: return uint32_t(0x117);
    case Qt::ExtraButton6: return uint32_t(0x118);
    case Qt::ExtraButton7: return uint32_t(0x119);
    case Qt::ExtraButton8: return uint32_t(0x11a);
    case Qt::ExtraButton9: return uint32_t(0x11b);
    case Qt::ExtraButton10: return uint32_t(0x11c);
    case Qt::ExtraButton11: return uint32_t(0x11d);
    case Qt::ExtraButton12: return uint32_t(0x11e);
    case Qt::ExtraButton13: return uint32_t(0x11f);
        // default should not occur; but if it does, then return Wayland's highest possible button number.
    default: return uint32_t(0x11f);
    }
}

DataDevice *InputDevice::dataDevice(struct wl_client *client) const
{
    for (int i = 0; i < m_data_devices.size();i++) {
        if (m_data_devices.at(i)->dataDeviceResource()->client == client) {
            return m_data_devices.at(i);
        }
    }
    return 0;
}

}

QT_END_NAMESPACE
