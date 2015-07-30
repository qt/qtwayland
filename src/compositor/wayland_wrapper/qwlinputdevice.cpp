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
#include <QtCompositor/QWaylandClient>

#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE

QWaylandInputDevicePrivate::QWaylandInputDevicePrivate(QWaylandInputDevice *inputdevice, QWaylandCompositor *compositor, QWaylandInputDevice::CapabilityFlags caps)
    : QObjectPrivate()
    , QtWaylandServer::wl_seat(compositor->waylandDisplay(), 3)
    , m_dragHandle(new QWaylandDrag(inputdevice))
    , m_compositor(compositor)
    , m_outputSpace(compositor->primaryOutputSpace())
    , m_capabilities(caps)
    , m_pointer(m_capabilities & QWaylandInputDevice::Pointer ? new QWaylandPointer(inputdevice) : 0)
    , m_keyboard(m_capabilities & QWaylandInputDevice::Keyboard ? new QWaylandKeyboard(inputdevice) : 0)
    , m_touch(m_capabilities & QWaylandInputDevice::Touch ? new QWaylandTouch(inputdevice) : 0)
    , m_inputMethod(m_compositor->extensionFlags() & QWaylandCompositor::TextInputExtension ? new QtWayland::InputMethod(m_compositor, inputdevice) : 0)
    , m_data_device()
{
}

QWaylandInputDevicePrivate::~QWaylandInputDevicePrivate()
{
}

QWaylandPointer *QWaylandInputDevicePrivate::pointerDevice() const
{
    return m_pointer.data();
}

QWaylandKeyboard *QWaylandInputDevicePrivate::keyboardDevice() const
{
    return m_keyboard.data();
}

QWaylandTouch *QWaylandInputDevicePrivate::touchDevice() const
{
    return m_touch.data();
}

QtWayland::InputMethod *QWaylandInputDevicePrivate::inputMethod()
{
    return m_inputMethod.data();
}


void QWaylandInputDevicePrivate::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void QWaylandInputDevicePrivate::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of m_capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)m_capabilities);
}

void QWaylandInputDevicePrivate::setCapabilities(QWaylandInputDevice::CapabilityFlags caps)
{
    Q_Q(QWaylandInputDevice);
    if (m_capabilities != caps) {
        QWaylandInputDevice::CapabilityFlags changed = caps ^ m_capabilities;

        if (changed & QWaylandInputDevice::Pointer) {
            m_pointer.reset(m_pointer.isNull() ? new QWaylandPointer(q) : 0);
        }

        if (changed & QWaylandInputDevice::Keyboard) {
            m_keyboard.reset(m_keyboard.isNull() ? new QWaylandKeyboard(q) : 0);
        }

        if (changed & QWaylandInputDevice::Touch) {
            m_touch.reset(m_touch.isNull() ? new QWaylandTouch(q) : 0);
        }

        m_capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)m_capabilities);
        }
    }
}

void QWaylandInputDevicePrivate::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_pointer.isNull()) {
        m_pointer->addClient(QWaylandClient::fromWlClient(resource->client()), id);
    }
}

void QWaylandInputDevicePrivate::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_keyboard.isNull()) {
        QWaylandKeyboardPrivate::get(m_keyboard.data())->add(resource->client(), id, resource->version());
    }
}

void QWaylandInputDevicePrivate::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!m_touch.isNull()) {
        m_touch->addClient(QWaylandClient::fromWlClient(resource->client()), id);
    }
}

void QWaylandInputDevicePrivate::sendMousePressEvent(Qt::MouseButton button)
{
    pointerDevice()->sendMousePressEvent(button);
}

void QWaylandInputDevicePrivate::sendMouseReleaseEvent(Qt::MouseButton button)
{
    pointerDevice()->sendMouseReleaseEvent(button);
}

void QWaylandInputDevicePrivate::sendMouseMoveEvent(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    pointerDevice()->sendMouseMoveEvent(surface, localPos,globalPos);
}

void QWaylandInputDevicePrivate::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    pointerDevice()->sendMouseWheelEvent(orientation, delta);
}

void QWaylandInputDevicePrivate::sendResetCurrentMouseView()
{
    pointerDevice()->resetCurrentView();
}

void QWaylandInputDevicePrivate::sendTouchPointEvent(int id, const QPointF &point, Qt::TouchPointState state)
{
    if (m_touch.isNull()) {
        return;
    }
    m_touch->sendTouchPointEvent(id, point,state);
}

void QWaylandInputDevicePrivate::sendTouchFrameEvent()
{
    if (!m_touch.isNull()) {
        m_touch->sendFrameEvent();
    }
}

void QWaylandInputDevicePrivate::sendTouchCancelEvent()
{
    if (!m_touch.isNull()) {
        m_touch->sendCancelEvent();
    }
}

void QWaylandInputDevicePrivate::sendFullKeyEvent(QKeyEvent *event)
{
    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    QtWayland::QtKeyExtensionGlobal *ext = QtWayland::QtKeyExtensionGlobal::get(m_compositor);
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()->handle()))
        return;

    if (!m_keyboard.isNull() && !event->isAutoRepeat()) {
        if (event->type() == QEvent::KeyPress)
            m_keyboard->sendKeyPressEvent(event->nativeScanCode());
        else if (event->type() == QEvent::KeyRelease)
            m_keyboard->sendKeyReleaseEvent(event->nativeScanCode());
    }
}

void QWaylandInputDevicePrivate::sendFullKeyEvent(QWaylandSurface *surface, QKeyEvent *event)
{
    QtWayland::QtKeyExtensionGlobal *ext = QtWayland::QtKeyExtensionGlobal::get(m_compositor);
    if (ext)
        ext->postQtKeyEvent(event, surface->handle());
}

void QWaylandInputDevicePrivate::sendFullTouchEvent(QTouchEvent *event)
{
    if (!mouseFocus()) {
        qWarning("Cannot send touch event, no pointer focus, fix the compositor");
        return;
    }

    if (!m_touch)
        return;

    m_touch->sendFullTouchEvent(event);
}

QWaylandSurface *QWaylandInputDevicePrivate::keyboardFocus() const
{
    if (m_keyboard.isNull() || !m_keyboard->focus())
        return Q_NULLPTR;

    return m_keyboard->focus();
}

/*!
 * \return True if the keyboard focus is changed successfully. False for inactive transient surfaces.
 */
bool QWaylandInputDevicePrivate::setKeyboardFocus(QWaylandSurface *surface)
{
    if (surface && (surface->transientInactive() || surface->handle()->isDestroyed()))
        return false;

    if (!m_keyboard.isNull()) {
        m_keyboard->setFocus(surface);
        if (m_data_device)
            m_data_device->setFocus(m_keyboard->focusClient());
        return true;
    }
    return false;
}

QWaylandSurfaceView *QWaylandInputDevicePrivate::mouseFocus() const
{
    return m_pointer.isNull() ? 0 : m_pointer->currentView();
}

void QWaylandInputDevicePrivate::clientRequestedDataDevice(QtWayland::DataDeviceManager *, struct wl_client *client, uint32_t id)
{
    Q_Q(QWaylandInputDevice);
    if (!m_data_device)
        m_data_device.reset(new QtWayland::DataDevice(q));
    m_data_device->add(client, id, 1);
}

QWaylandCompositor *QWaylandInputDevicePrivate::compositor() const
{
    return m_compositor;
}

QWaylandDrag *QWaylandInputDevicePrivate::dragHandle() const
{
    return m_dragHandle.data();
}

const QtWayland::DataDevice *QWaylandInputDevicePrivate::dataDevice() const
{
    return m_data_device.data();
}

QWaylandOutputSpace *QWaylandInputDevicePrivate::outputSpace() const
{
    return m_outputSpace;
}

void QWaylandInputDevicePrivate::setOutputSpace(QWaylandOutputSpace *outputSpace)
{
    m_outputSpace = outputSpace;
}

QWaylandInputDevicePrivate *QWaylandInputDevicePrivate::get(QWaylandInputDevice *device)
{
    return device->d_func();
}

QT_END_NAMESPACE
