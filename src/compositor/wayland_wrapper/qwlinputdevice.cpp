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

#include "qwlinputdevice_p.h"

#include "qwlcompositor_p.h"
#include "qwldatadevice_p.h"
#include "qwlinputmethod_p.h"
#include "qwlsurface_p.h"
#include "qwlqttouch_p.h"
#include "qwlqtkey_p.h"
#include "qwaylandcompositor.h"
#include "qwaylanddrag.h"
#include "qwlpointer_p.h"
#include "qwlkeyboard_p.h"
#include "qwltouch_p.h"
#include "qwaylandsurfaceview.h"

#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE

namespace QtWayland {

InputDevice::InputDevice(QWaylandInputDevice *handle, Compositor *compositor, QWaylandInputDevice::CapabilityFlags caps)
    : QtWaylandServer::wl_seat(compositor->wl_display(), 3)
    , m_handle(handle)
    , m_dragHandle(new QWaylandDrag(this))
    , m_compositor(compositor)
    , m_capabilities(caps)
    , m_pointer(m_capabilities & QWaylandInputDevice::Pointer ? new Pointer(m_compositor, this) : 0)
    , m_keyboard(m_capabilities & QWaylandInputDevice::Keyboard ? new Keyboard(m_compositor, this) : 0)
    , m_touch(m_capabilities & QWaylandInputDevice::Touch ? new Touch(m_compositor) : 0)
    , m_inputMethod(m_compositor->extensions() & QWaylandCompositor::TextInputExtension ? new InputMethod(m_compositor, this) : 0)
    , m_data_device()
{
}

InputDevice::~InputDevice()
{
}

Pointer *InputDevice::pointerDevice()
{
    return m_pointer.data();
}

Keyboard *InputDevice::keyboardDevice()
{
    return m_keyboard.data();
}

Touch *InputDevice::touchDevice()
{
    return m_touch.data();
}

InputMethod *InputDevice::inputMethod()
{
    return m_inputMethod.data();
}

const Pointer *InputDevice::pointerDevice() const
{
    return m_pointer.data();
}

const Keyboard *InputDevice::keyboardDevice() const
{
    return m_keyboard.data();
}

const Touch *InputDevice::touchDevice() const
{
    return m_touch.data();
}

void InputDevice::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void InputDevice::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of m_capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)m_capabilities);
}

void InputDevice::setCapabilities(QWaylandInputDevice::CapabilityFlags caps)
{
    if (m_capabilities != caps) {
        QWaylandInputDevice::CapabilityFlags changed = caps ^ m_capabilities;

        if (changed & QWaylandInputDevice::Pointer) {
            m_pointer.reset(m_pointer.isNull() ? new Pointer(m_compositor, this) : 0);
        }

        if (changed & QWaylandInputDevice::Keyboard) {
            m_keyboard.reset(m_keyboard.isNull() ? new Keyboard(m_compositor, this) : 0);
        }

        if (changed & QWaylandInputDevice::Touch) {
            m_touch.reset(m_touch.isNull() ? new Touch(m_compositor) : 0);
        }

        m_capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)m_capabilities);
        }
    }
}

void InputDevice::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_pointer.isNull()) {
        m_pointer->add(resource->client(), id, resource->version());
    }
}

void InputDevice::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_keyboard.isNull()) {
        m_keyboard->add(resource->client(), id, resource->version());
    }
}

void InputDevice::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_touch.isNull()) {
        m_touch->add(resource->client(), id, resource->version());
    }
}

void InputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMousePressEvent(button, localPos, globalPos);
}

void InputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMouseReleaseEvent(button, localPos, globalPos);
}

void InputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMouseMoveEvent(localPos, globalPos);
}

void InputDevice::sendMouseMoveEvent(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    setMouseFocus(surface,localPos,globalPos);
    sendMouseMoveEvent(localPos,globalPos);
}

void InputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    pointerDevice()->sendMouseWheelEvent(orientation, delta);
}

void InputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    if (m_touch.isNull()) {
        return;
    }

    switch (state) {
    case Qt::TouchPointPressed:
        m_touch->sendDown(id, QPointF(x, y));
        break;
    case Qt::TouchPointMoved:
        m_touch->sendMotion(id, QPointF(x, y));
        break;
    case Qt::TouchPointReleased:
        m_touch->sendUp(id);
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
    if (!m_touch.isNull()) {
        m_touch->sendFrame();
    }
}

void InputDevice::sendTouchCancelEvent()
{
    if (!m_touch.isNull()) {
        m_touch->sendCancel();
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

    if (!m_keyboard.isNull() && !event->isAutoRepeat()) {
        if (event->type() == QEvent::KeyPress)
            m_keyboard->sendKeyPressEvent(event->nativeScanCode());
        else if (event->type() == QEvent::KeyRelease)
            m_keyboard->sendKeyReleaseEvent(event->nativeScanCode());
    }
}

void InputDevice::sendFullKeyEvent(Surface *surface, QKeyEvent *event)
{
    QtKeyExtensionGlobal *ext = m_compositor->qtkeyExtension();
    if (ext)
        ext->postQtKeyEvent(event, surface);
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
    return m_keyboard.isNull() ? 0 : m_keyboard->focus();
}

/*!
 * \return True if the keyboard focus is changed successfully. False for inactive transient surfaces.
 */
bool InputDevice::setKeyboardFocus(Surface *surface)
{
    if (surface && (surface->transientInactive() || surface->isDestroyed()))
        return false;

    if (!m_keyboard.isNull()) {
        m_keyboard->setFocus(surface);
        if (m_data_device)
            m_data_device->setFocus(m_keyboard->focusResource());
        return true;
    }
    return false;
}

QWaylandSurfaceView *InputDevice::mouseFocus() const
{
    return m_pointer.isNull() ? 0 : m_pointer->focusSurface();
}

void InputDevice::setMouseFocus(QWaylandSurfaceView *view, const QPointF &localPos, const QPointF &globalPos)
{
    if (view && view->surface()->handle()->isDestroyed())
        return;

    if (!m_pointer.isNull()) {
        m_pointer->setMouseFocus(view, localPos, globalPos);
    }

    if (!m_touch.isNull()) {
        // We have no separate touch focus management so make it match the pointer focus always.
        // No wl_touch_set_focus() is available so set it manually.
        m_touch->setFocus(view);
    }
}

void InputDevice::clientRequestedDataDevice(DataDeviceManager *, struct wl_client *client, uint32_t id)
{
    if (!m_data_device)
        m_data_device.reset(new DataDevice(this));
    m_data_device->add(client, id, 1);
}

Compositor *InputDevice::compositor() const
{
    return m_compositor;
}

QWaylandInputDevice *InputDevice::handle() const
{
    return m_handle;
}

QWaylandDrag *InputDevice::dragHandle() const
{
    return m_dragHandle.data();
}

const DataDevice *InputDevice::dataDevice() const
{
    return m_data_device.data();
}

}

QT_END_NAMESPACE
