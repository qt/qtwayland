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

#include "qwaylandinput.h"
#include "qwaylandinput_p.h"

#include "qwaylandcompositor.h"
#include "qwaylandview.h"
#include <QtCompositor/QWaylandDrag>
#include <QtCompositor/QWaylandTouch>
#include <QtCompositor/private/qwlinputmethod_p.h>
#include <QtCompositor/private/qwaylandinput_p.h>
#include <QtCompositor/private/qwaylandcompositor_p.h>
#include <QtCompositor/private/qwldatadevice_p.h>

#include "extensions/qwlqtkey_p.h"

QT_BEGIN_NAMESPACE

QWaylandInputDevicePrivate::QWaylandInputDevicePrivate(QWaylandInputDevice *inputdevice, QWaylandCompositor *compositor)
    : QObjectPrivate()
    , QtWaylandServer::wl_seat(compositor->display(), 3)
    , compositor(compositor)
    , outputSpace(compositor->primaryOutputSpace())
    , mouseFocus(Q_NULLPTR)
    , capabilities()
    , input_method(compositor->extensionFlags() & QWaylandCompositor::TextInputExtension ? new QtWayland::InputMethod(compositor, inputdevice) : 0)
    , data_device()
    , drag_handle(new QWaylandDrag(inputdevice))
{
}

QWaylandInputDevicePrivate::~QWaylandInputDevicePrivate()
{
}

void QWaylandInputDevicePrivate::setCapabilities(QWaylandInputDevice::CapabilityFlags caps)
{
    Q_Q(QWaylandInputDevice);
    if (capabilities != caps) {
        QWaylandInputDevice::CapabilityFlags changed = caps ^ capabilities;

        if (changed & QWaylandInputDevice::Pointer) {
            pointer.reset(pointer.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreatePointerDevice(q) : 0);
        }

        if (changed & QWaylandInputDevice::Keyboard) {
            keyboard.reset(keyboard.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreateKeyboardDevice(q) : 0);
        }

        if (changed & QWaylandInputDevice::Touch) {
            touch.reset(touch.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreateTouchDevice(q) : 0);
        }

        capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)capabilities);
        }
    }
}

void QWaylandInputDevicePrivate::clientRequestedDataDevice(QtWayland::DataDeviceManager *, struct wl_client *client, uint32_t id)
{
    Q_Q(QWaylandInputDevice);
    if (!data_device)
        data_device.reset(new QtWayland::DataDevice(q));
    data_device->add(client, id, 1);
}

void QWaylandInputDevicePrivate::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void QWaylandInputDevicePrivate::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)capabilities);
}

void QWaylandInputDevicePrivate::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!pointer.isNull()) {
        pointer->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id);
    }
}

void QWaylandInputDevicePrivate::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!keyboard.isNull()) {
        keyboard->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id);
    }
}

void QWaylandInputDevicePrivate::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!touch.isNull()) {
        touch->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id);
    }
}

QWaylandKeymap::QWaylandKeymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules)
              : m_layout(layout)
              , m_variant(variant)
              , m_options(options)
              , m_rules(rules)
              , m_model(model)
{
}



QWaylandInputDevice::QWaylandInputDevice(QWaylandCompositor *compositor, CapabilityFlags caps)
    : QObject(*new QWaylandInputDevicePrivate(this,compositor))
{
    d_func()->setCapabilities(caps);
}

QWaylandInputDevice::~QWaylandInputDevice()
{
}

void QWaylandInputDevice::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMousePressEvent(button);
}

void QWaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseReleaseEvent(button);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void QWaylandInputDevice::sendMouseMoveEvent(QWaylandView *view, const QPointF &localPos, const QPointF &globalPos)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseMoveEvent(view, localPos,globalPos);
}

void QWaylandInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseWheelEvent(orientation, delta);
}

void QWaylandInputDevice::sendKeyPressEvent(uint code)
{
    Q_D(QWaylandInputDevice);
    d->keyboard->sendKeyPressEvent(code);
}

void QWaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    Q_D(QWaylandInputDevice);
    d->keyboard->sendKeyReleaseEvent(code);
}

void QWaylandInputDevice::sendTouchPointEvent(int id, const QPointF &point, Qt::TouchPointState state)
{
    Q_D(QWaylandInputDevice);
    if (d->touch.isNull()) {
        return;
    }
    d->touch->sendTouchPointEvent(id, point,state);
}

void QWaylandInputDevice::sendTouchFrameEvent()
{
    Q_D(QWaylandInputDevice);
    if (!d->touch.isNull()) {
        d->touch->sendFrameEvent();
    }
}

void QWaylandInputDevice::sendTouchCancelEvent()
{
    Q_D(QWaylandInputDevice);
    if (!d->touch.isNull()) {
        d->touch->sendCancelEvent();
    }
}

void QWaylandInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    Q_D(QWaylandInputDevice);
    if (!mouseFocus()) {
        qWarning("Cannot send touch event, no pointer focus, fix the compositor");
        return;
    }

    if (!d->touch)
        return;

    d->touch->sendFullTouchEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandInputDevice);
    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    QtWayland::QtKeyExtensionGlobal *ext = QtWayland::QtKeyExtensionGlobal::findIn(d->compositor);
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (!d->keyboard.isNull() && !event->isAutoRepeat()) {
        if (event->type() == QEvent::KeyPress)
            d->keyboard->sendKeyPressEvent(event->nativeScanCode());
        else if (event->type() == QEvent::KeyRelease)
            d->keyboard->sendKeyReleaseEvent(event->nativeScanCode());
    }
}

QWaylandKeyboard *QWaylandInputDevice::keyboard() const
{
    Q_D(const QWaylandInputDevice);
    return d->keyboard.data();
}

QWaylandSurface *QWaylandInputDevice::keyboardFocus() const
{
    Q_D(const QWaylandInputDevice);
    if (d->keyboard.isNull() || !d->keyboard->focus())
        return Q_NULLPTR;

    return d->keyboard->focus();
}

bool QWaylandInputDevice::setKeyboardFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandInputDevice);
    if (surface && surface->isDestroyed())
        return false;

    if (!d->keyboard.isNull() && d->keyboard->setFocus(surface)) {
        if (d->data_device)
            d->data_device->setFocus(d->keyboard->focusClient());
        return true;
    }
    return false;
}

void QWaylandInputDevice::setKeymap(const QWaylandKeymap &keymap)
{
    if (keyboard())
        keyboard()->setKeymap(keymap);
}

QWaylandPointer *QWaylandInputDevice::pointer() const
{
    Q_D(const QWaylandInputDevice);
    return d->pointer.data();
}

QWaylandView *QWaylandInputDevice::mouseFocus() const
{
    Q_D(const QWaylandInputDevice);
    return d->mouseFocus;
}

void QWaylandInputDevice::setMouseFocus(QWaylandView *view)
{
    Q_D(QWaylandInputDevice);
    if (view == d->mouseFocus)
        return;

    QWaylandView *oldFocus = d->mouseFocus;
    d->mouseFocus = view;
    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}

QWaylandOutputSpace *QWaylandInputDevice::outputSpace() const
{
    Q_D(const QWaylandInputDevice);
    return d->outputSpace;
}

void QWaylandInputDevice::setOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandInputDevice);
    d->outputSpace = outputSpace;
}

QWaylandCompositor *QWaylandInputDevice::compositor() const
{
    Q_D(const QWaylandInputDevice);
    return d->compositor;
}

QWaylandDrag *QWaylandInputDevice::drag() const
{
    Q_D(const QWaylandInputDevice);
    return d->drag_handle.data();
}

QWaylandInputDevice::CapabilityFlags QWaylandInputDevice::capabilities() const
{
    Q_D(const QWaylandInputDevice);
    return d->capabilities;
}

bool QWaylandInputDevice::isOwner(QInputEvent *inputEvent) const
{
    Q_UNUSED(inputEvent);
    return true;
}

QWaylandInputDevice *QWaylandInputDevice::fromSeatResource(struct ::wl_resource *resource)
{
    return static_cast<QWaylandInputDevicePrivate *>(QWaylandInputDevicePrivate::Resource::fromResource(resource)->seat_object)->q_func();
}

QT_END_NAMESPACE
