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
#include "qwaylandinputmethodcontrol.h"
#include "qwaylandview.h"
#include <QtWaylandCompositor/QWaylandDrag>
#include <QtWaylandCompositor/QWaylandTouch>
#include <QtWaylandCompositor/QWaylandPointer>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/private/qwaylandinput_p.h>
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwldatadevice_p.h>

#include "extensions/qwlqtkey_p.h"
#include "extensions/qwaylandtextinput.h"

QT_BEGIN_NAMESPACE

QWaylandInputDevicePrivate::QWaylandInputDevicePrivate(QWaylandInputDevice *inputdevice, QWaylandCompositor *compositor)
    : QObjectPrivate()
    , QtWaylandServer::wl_seat(compositor->display(), 4)
    , compositor(compositor)
    , mouseFocus(Q_NULLPTR)
    , keyboardFocus(nullptr)
    , capabilities()
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

        if ((changed & caps & QWaylandInputDevice::Keyboard) && keyboardFocus != nullptr)
            keyboard->setFocus(keyboardFocus);
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
        pointer->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void QWaylandInputDevicePrivate::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!keyboard.isNull()) {
        keyboard->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void QWaylandInputDevicePrivate::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!touch.isNull()) {
        touch->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
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


/*!
 * \class QWaylandInputDevice
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief The QWaylandInputDevice class provides access to keyboard, mouse and touch input.
 *
 * The QWaylandInputDevice provides access to different types of user input and maintains
 * a keyboard focus and a mouse pointer. It corresponds to the wl_seat interface in the Wayland protocol.
 */

/*!
 * \enum QWaylandInputDevice::CapabilityFlag
 *
 * This enum type describes the capabilities of a QWaylandInputDevice.
 *
 * \value Pointer The QWaylandInputDevice supports pointer input.
 * \value Keyboard The QWaylandInputDevice supports keyboard input.
 * \value Touch The QWaylandInputDevice supports touch input.
 */

/*!
 * Constructs a QWaylandInputDevice for the given \a compositor and with the given \a capabilityFlags.
 */
QWaylandInputDevice::QWaylandInputDevice(QWaylandCompositor *compositor, CapabilityFlags capabilityFlags)
    : QWaylandObject(*new QWaylandInputDevicePrivate(this,compositor))
{
    d_func()->setCapabilities(capabilityFlags);
}

/*!
 * Destroys the QWaylandInputDevice
 */
QWaylandInputDevice::~QWaylandInputDevice()
{
}

/*!
 * Sends a mouse press event for \a button to the QWaylandInputDevice's pointer device.
 */
void QWaylandInputDevice::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMousePressEvent(button);
}

/*!
 * Sends a mouse release event for \a button to the QWaylandInputDevice's pointer device.
 */
void QWaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseReleaseEvent(button);
}

/*!
 * Sets the mouse focus to \a view and sends a mouse move event to the pointer device with the
 * local position \a localPos and output space position \a outputSpacePos.
 **/
void QWaylandInputDevice::sendMouseMoveEvent(QWaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseMoveEvent(view, localPos, outputSpacePos);
}

/*!
 * Sends a mouse wheel event to the QWaylandInputDevice's pointer device with the given \a orientation and \a delta.
 */
void QWaylandInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandInputDevice);
    d->pointer->sendMouseWheelEvent(orientation, delta);
}

/*!
 * Sends a key press event with the key \a code to the keyboard device.
 */
void QWaylandInputDevice::sendKeyPressEvent(uint code)
{
    Q_D(QWaylandInputDevice);
    d->keyboard->sendKeyPressEvent(code);
}

/*!
 * Sends a key release event with the key \a code to the keyboard device.
 */
void QWaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    Q_D(QWaylandInputDevice);
    d->keyboard->sendKeyReleaseEvent(code);
}

/*!
 * Sends a touch point event with the given \a id and \a state to the touch device. The position
 * of the touch point is given by \a point.
 */
void QWaylandInputDevice::sendTouchPointEvent(int id, const QPointF &point, Qt::TouchPointState state)
{
    Q_D(QWaylandInputDevice);
    if (d->touch.isNull()) {
        return;
    }
    d->touch->sendTouchPointEvent(id, point,state);
}

/*!
 * Sends a frame event to the touch device.
 */
void QWaylandInputDevice::sendTouchFrameEvent()
{
    Q_D(QWaylandInputDevice);
    if (!d->touch.isNull()) {
        d->touch->sendFrameEvent();
    }
}

/*!
 * Sends a cancel event to the touch device.
 */
void QWaylandInputDevice::sendTouchCancelEvent()
{
    Q_D(QWaylandInputDevice);
    if (!d->touch.isNull()) {
        d->touch->sendCancelEvent();
    }
}

/*!
 * Sends the \a event to the touch device.
 */
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

/*!
 * Sends the \a event to the keyboard device.
 */
void QWaylandInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandInputDevice);

    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    if (keyboardFocus()->inputMethodControl()->enabled()
        && event->nativeScanCode() == 0) {
        QWaylandTextInput *textInput = QWaylandTextInput::findIn(this);
        if (textInput) {
            textInput->sendKeyEvent(event);
            return;
        }
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

/*!
 * Returns the keyboard for this input device.
 */
QWaylandKeyboard *QWaylandInputDevice::keyboard() const
{
    Q_D(const QWaylandInputDevice);
    return d->keyboard.data();
}

/*!
 * Returns the current focused surface for keyboard input.
 */
QWaylandSurface *QWaylandInputDevice::keyboardFocus() const
{
    Q_D(const QWaylandInputDevice);
    if (d->keyboard.isNull() || !d->keyboard->focus())
        return Q_NULLPTR;

    return d->keyboard->focus();
}

/*!
 * Sets the current keyboard focus to \a surface.
 */
bool QWaylandInputDevice::setKeyboardFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandInputDevice);
    if (surface && surface->isDestroyed())
        return false;

    QWaylandSurface *oldSurface = keyboardFocus();
    if (surface == oldSurface)
        return true;

    QWaylandWlShellSurface *wlShellsurface = QWaylandWlShellSurface::findIn(surface);
    if (wlShellsurface && wlShellsurface->focusPolicy() == QWaylandWlShellSurface::NoKeyboardFocus)
        return false;

    d->keyboardFocus = surface;
    if (!d->keyboard.isNull())
        d->keyboard->setFocus(surface);
    if (d->data_device)
        d->data_device->setFocus(surface->client());
    emit keyboardFocusChanged(surface, oldSurface);
    return true;
}

/*!
 * Sets the key map of this QWaylandInputDevice to \a keymap.
 */
void QWaylandInputDevice::setKeymap(const QWaylandKeymap &keymap)
{
    if (keyboard())
        keyboard()->setKeymap(keymap);
}

/*!
 * Returns the pointer device for this QWaylandInputDevice.
 */
QWaylandPointer *QWaylandInputDevice::pointer() const
{
    Q_D(const QWaylandInputDevice);
    return d->pointer.data();
}

/*!
 * Returns the touch device for this QWaylandInputDevice.
 */
QWaylandTouch *QWaylandInputDevice::touch() const
{
    Q_D(const QWaylandInputDevice);
    return d->touch.data();
}

/*!
 * Returns the view that currently has mouse focus.
 */
QWaylandView *QWaylandInputDevice::mouseFocus() const
{
    Q_D(const QWaylandInputDevice);
    return d->mouseFocus;
}

/*!
 * Sets the current mouse focus to \a view.
 */
void QWaylandInputDevice::setMouseFocus(QWaylandView *view)
{
    Q_D(QWaylandInputDevice);
    if (view == d->mouseFocus)
        return;

    QWaylandView *oldFocus = d->mouseFocus;
    d->mouseFocus = view;
    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}

/*!
 * Returns the compositor for this QWaylandInputDevice.
 */
QWaylandCompositor *QWaylandInputDevice::compositor() const
{
    Q_D(const QWaylandInputDevice);
    return d->compositor;
}

/*!
 * Returns the drag object for this QWaylandInputDevice.
 */
QWaylandDrag *QWaylandInputDevice::drag() const
{
    Q_D(const QWaylandInputDevice);
    return d->drag_handle.data();
}

/*!
 * Returns the capability flags for this QWaylandInputDevice.
 */
QWaylandInputDevice::CapabilityFlags QWaylandInputDevice::capabilities() const
{
    Q_D(const QWaylandInputDevice);
    return d->capabilities;
}

/*!
 * \internal
 */
bool QWaylandInputDevice::isOwner(QInputEvent *inputEvent) const
{
    Q_UNUSED(inputEvent);
    return true;
}

/*!
 * Returns the QWaylandInputDevice corresponding to the \a resource. The \a resource is expected
 * to have the type wl_seat.
 */
QWaylandInputDevice *QWaylandInputDevice::fromSeatResource(struct ::wl_resource *resource)
{
    return static_cast<QWaylandInputDevicePrivate *>(QWaylandInputDevicePrivate::Resource::fromResource(resource)->seat_object)->q_func();
}

/*!
 * \fn void mouseFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus)
 *
 * This signal is emitted when the mouse focus has changed from \a oldFocus to \a newFocus.
 */

QT_END_NAMESPACE
