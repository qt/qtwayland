/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#include "wlinputdevice.h"

#include "wlshmbuffer.h"
#include "wlcompositor.h"
#include "wldatadevice.h"
#include "wlsurface.h"
#include "wltouch.h"
#include "wlqtkey.h"
#include "waylandcompositor.h"

#include <QtGui/QTouchEvent>

namespace Wayland {

static ShmBuffer *currentCursor;

InputDevice::InputDevice(WaylandInputDevice *handle, Compositor *compositor)
    : m_handle(handle)
    , m_compositor(compositor)
{
    wl_input_device_init(base());
    wl_display_add_global(compositor->wl_display(),&wl_input_device_interface,this,InputDevice::bind_func);
}

InputDevice::~InputDevice()
{
    qDeleteAll(m_data_devices);
}

void InputDevice::sendMousePressEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);

    uint32_t time = m_compositor->currentTimeMsecs();
    struct wl_resource *pointer_focus_resource = base()->pointer_focus_resource;
    if (pointer_focus_resource) {
        wl_input_device_send_button(pointer_focus_resource,
                                    time, toWaylandButton(button), 1);
    }
}

void InputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);

    uint32_t time = m_compositor->currentTimeMsecs();
    struct wl_resource *pointer_focus_resource = base()->pointer_focus_resource;
    if (pointer_focus_resource) {
        wl_input_device_send_button(pointer_focus_resource,
                                    time, toWaylandButton(button), 0);
    }
}

void InputDevice::sendMouseMoveEvent(const QPoint &localPos, const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
    uint32_t time = m_compositor->currentTimeMsecs();
    struct wl_resource *pointer_focus_resource = base()->pointer_focus_resource;
    if (pointer_focus_resource) {
        wl_input_device_send_motion(pointer_focus_resource,
                                    time,
                                    localPos.x(), localPos.y());
    }
}

void InputDevice::sendMouseMoveEvent(Surface *surface, const QPoint &localPos, const QPoint &globalPos)
{
    if (mouseFocus() != surface) {
        setMouseFocus(surface,localPos,globalPos);
    }
    sendMouseMoveEvent(localPos,globalPos);
}

void InputDevice::sendKeyPressEvent(uint code)
{
    if (base()->keyboard_focus_resource != NULL) {
        uint32_t time = m_compositor->currentTimeMsecs();
        wl_input_device_send_key(base()->keyboard_focus_resource,
                                 time, code - 8, 1);
    }
}

void InputDevice::sendKeyReleaseEvent(uint code)
{
    if (base()->keyboard_focus_resource != NULL) {
        uint32_t time = m_compositor->currentTimeMsecs();
        wl_input_device_send_key(base()->keyboard_focus_resource,
                                 time, code - 8, 0);
    }
}

void InputDevice::sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state)
{
    uint32_t time = m_compositor->currentTimeMsecs();
    struct wl_resource *resource = base()->pointer_focus_resource;
    if (!resource)
        return;
    switch (state) {
    case Qt::TouchPointPressed:
        wl_input_device_send_touch_down(resource, time, &base()->pointer_focus->resource, id, x, y);
        break;
    case Qt::TouchPointMoved:
        wl_input_device_send_touch_motion(resource, time, id, x, y);
        break;
    case Qt::TouchPointReleased:
        wl_input_device_send_touch_up(resource, time, id);
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
    struct wl_resource *resource = base()->pointer_focus_resource;
    if (resource) {
        wl_input_device_send_touch_frame(resource);
    }
}

void InputDevice::sendTouchCancelEvent()
{
    struct wl_resource *resource = base()->pointer_focus_resource;
    if (resource) {
        wl_input_device_send_touch_cancel(resource);
    }
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
        QPoint p = (tp.pos() - pos).toPoint();
        sendTouchPointEvent(tp.id(), p.x(), p.y(), tp.state());
    }
    sendTouchFrameEvent();
}

Surface *InputDevice::keyboardFocus() const
{
    return wayland_cast<Surface>(base()->keyboard_focus);
}

void InputDevice::setKeyboardFocus(Surface *surface)
{
    sendSelectionFocus(surface);
    wl_input_device_set_keyboard_focus(base(), surface ? surface->base() : 0, m_compositor->currentTimeMsecs());
}

Surface *InputDevice::mouseFocus() const
{
    return wayland_cast<Surface>(base()->pointer_focus);
}

void InputDevice::setMouseFocus(Surface *surface, const QPoint &globalPos, const QPoint &localPos)
{
    Q_UNUSED(globalPos);
    wl_input_device_set_pointer_focus(base(),
                                      surface ? surface->base() : 0,
                                      m_compositor->currentTimeMsecs(),
                                      localPos.x(), localPos.y());
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
    DataDevice *device = dataDevice(surface->base()->resource.client);
    if (device) {
        device->sendSelectionFocus();
    }
}

Compositor *InputDevice::compositor() const
{
    return m_compositor;
}

WaylandInputDevice *InputDevice::handle() const
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

void InputDevice::bind_func(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    struct wl_resource *resource = wl_client_add_object(client,&wl_input_device_interface ,&input_device_interface,id,data);

    struct wl_input_device *input_device = static_cast<struct wl_input_device *>(data);
    resource->destroy = destroy_resource;
    wl_list_insert(&input_device->resource_list,&resource->link);
}

void InputDevice::input_device_attach(struct wl_client *client,
                         struct wl_resource *device_resource,
                         uint32_t time,
                         struct wl_resource *buffer_resource, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    Q_UNUSED(time);

    struct wl_input_device *device_base = reinterpret_cast<struct wl_input_device *>(device_resource->data);
    struct wl_buffer *buffer = reinterpret_cast<struct wl_buffer *>(buffer_resource);

    InputDevice *inputDevice = wayland_cast<InputDevice>(device_base);
    if (wl_buffer_is_shm(buffer)) {
        ShmBuffer *shmBuffer = static_cast<ShmBuffer *>(buffer->user_data);
        if (shmBuffer) {
            inputDevice->m_compositor->waylandCompositor()->changeCursor(shmBuffer->image(), x, y);
            currentCursor = shmBuffer;
        }
    }
}

const struct wl_input_device_interface InputDevice::input_device_interface = {
    InputDevice::input_device_attach,
};

void InputDevice::destroy_resource(wl_resource *resource)
{
    InputDevice *input_device = static_cast<InputDevice *>(resource->data);
    if (input_device->base()->keyboard_focus_resource == resource) {
        input_device->base()->keyboard_focus_resource = 0;
    }
    if (input_device->base()->pointer_focus_resource == resource) {
        input_device->base()->pointer_focus_resource = 0;
    }

    input_device->cleanupDataDeviceForClient(resource->client, true);

    wl_list_remove(&resource->link);

    free(resource);
}

}
