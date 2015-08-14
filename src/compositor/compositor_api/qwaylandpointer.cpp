/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
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

#include "qwaylandpointer.h"
#include "qwaylandpointer_p.h"
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandCompositor>

QT_BEGIN_NAMESPACE

QWaylandPointerPrivate::QWaylandPointerPrivate(QWaylandPointer *pointer, QWaylandInputDevice *seat)
    : QObjectPrivate()
    , wl_pointer()
    , seat(seat)
    , output()
    , defaultGrab(pointer)
    , grab(&defaultGrab)
    , grabButton()
    , grabTime()
    , grabSerial()
    , focusResource()
    , hasSentEnter(false)
    , buttonCount()
{
}

void QWaylandPointerPrivate::pointer_destroy_resource(wl_pointer::Resource *resource)
{
    if (focusResource == resource->handle)
        focusResource = 0;
}

void QWaylandPointerPrivate::pointer_release(wl_pointer::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

static void requestCursorSurface(QWaylandCompositor *compositor, QWaylandSurface *surface, int32_t hotspot_x, int hotspot_y)
{
    compositor->currentCurserSurfaceRequest(surface, hotspot_x, hotspot_y);
}

void QWaylandPointerPrivate::pointer_set_cursor(wl_pointer::Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    if (!surface) {
        requestCursorSurface(compositor(), Q_NULLPTR, 0, 0);
        return;
    }

    QWaylandSurface *s = QWaylandSurface::fromResource(surface);
    s->markAsCursorSurface(true);
    requestCursorSurface(compositor(), s, hotspot_x, hotspot_y);
}

QWaylandPointerGrabber::~QWaylandPointerGrabber()
{
}

QWaylandPointer::QWaylandPointer(QWaylandInputDevice *seat, QObject *parent)
    : QObject(* new QWaylandPointerPrivate(this, seat), parent)
{
    connect(&d_func()->focusDestroyListener, &QWaylandDestroyListener::fired, this, &QWaylandPointer::focusDestroyed);
    connect(seat, &QWaylandInputDevice::mouseFocusChanged, this, &QWaylandPointer::pointerFocusChanged);
}

void QWaylandDefaultPointerGrabber::focus()
{
//    if (buttonPressed())
//        return;
//
//    setFocus(currentPosition.view(), currentPosition.position());
}

void QWaylandDefaultPointerGrabber::motion(uint32_t time)
{
    struct wl_resource *focusResource = pointer->focusResource();
    if (focusResource) {
        wl_fixed_t x = wl_fixed_from_double(pointer->currentLocalPosition().x());
        wl_fixed_t y = wl_fixed_from_double(pointer->currentLocalPosition().y());
        wl_pointer_send_motion(focusResource, time, x, y);
    }
}

void QWaylandDefaultPointerGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    struct wl_resource *focusResource = pointer->focusResource();
    if (focusResource) {
        pointer->sendButton(focusResource, time, button, state);
    }
}

QWaylandInputDevice *QWaylandPointer::inputDevice() const
{
    Q_D(const QWaylandPointer);
    return d->seat;
}

QWaylandCompositor *QWaylandPointer::compositor() const
{
    Q_D(const QWaylandPointer);
    return d->compositor();
}

QWaylandOutput *QWaylandPointer::output() const
{
    Q_D(const QWaylandPointer);
    return d->output;
}

void QWaylandPointer::setOutput(QWaylandOutput *output)
{
    Q_D(QWaylandPointer);
    if (d->output == output) return;
    d->output = output;
    outputChanged();
}

void QWaylandPointer::startGrab(QWaylandPointerGrabber *grab)
{
    Q_D(QWaylandPointer);
    d->grab = grab;
    d->grab->pointer = this;

    if (d->seat->mouseFocus())
        d->grab->focus();
}

void QWaylandPointer::endGrab()
{
    Q_D(QWaylandPointer);
    d->grab = &d->defaultGrab;
    d->grab->focus();
}

QWaylandPointerGrabber *QWaylandPointer::currentGrab() const
{
    Q_D(const QWaylandPointer);
    return d->grab;
}

Qt::MouseButton QWaylandPointer::grabButton() const
{
    Q_D(const QWaylandPointer);
    return d->grabButton;
}

uint32_t QWaylandPointer::grabTime() const
{
    Q_D(const QWaylandPointer);
    return d->grabTime;

}

uint32_t QWaylandPointer::grabSerial() const
{
    Q_D(const QWaylandPointer);
    return d->grabSerial;
}

void QWaylandPointer::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandPointer);
    uint32_t time = d->compositor()->currentTimeMsecs();
    if (d->buttonCount == 0) {
        d->grabButton = button;
        d->grabTime = time;
    }
    d->buttonCount++;
    d->grab->button(time, button, WL_POINTER_BUTTON_STATE_PRESSED);

    if (d->buttonCount == 1) {
        d->grabSerial = d->compositor()->nextSerial();
        buttonPressedChanged();
    }
}

void QWaylandPointer::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandPointer);
    uint32_t time = d->compositor()->currentTimeMsecs();
    d->buttonCount--;
    d->grab->button(time, button, WL_POINTER_BUTTON_STATE_RELEASED);

    if (d->buttonCount == 1)
        d->grabSerial = d->compositor()->nextSerial();
    if (d->buttonCount == 0)
        buttonPressedChanged();
}

void QWaylandPointer::sendMouseMoveEvent(QWaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(QWaylandPointer);
    if (view && (!view->surface() || view->surface()->isCursorSurface()))
        view = Q_NULLPTR;
    d->seat->setMouseFocus(view);
    d->localPosition = localPos;
    d->spacePosition = outputSpacePos;

    //we adjust if the mouse position is on the edge
    //to work around Qt's event propogation
    if (view && view->surface()) {
        QSizeF size(view->surface()->size());
        if (d->localPosition.x() ==  size.width())
            d->localPosition.rx() -= 0.01;

        if (d->localPosition.y() == size.height())
            d->localPosition.ry() -= 0.01;
    }

    QWaylandPointerPrivate::Resource *resource = view ? d->resourceMap().value(view->surface()->waylandClient()) : 0;
    if (resource && !d->hasSentEnter) {
        uint32_t serial = d->compositor()->nextSerial();
        QWaylandKeyboard *keyboard = d->seat->keyboard();
        if (keyboard) {
            keyboard->sendKeyModifiers(view->surface()->client(), serial);
        }
        d->send_enter(resource->handle, serial, view->surface()->resource(),
                   wl_fixed_from_double(d->localPosition.x()), wl_fixed_from_double(d->localPosition.y()));

        d->focusDestroyListener.listenForDestruction(view->surface()->resource());
        d->hasSentEnter = true;
    }

    d->focusResource = resource ? resource->handle : 0;

    if (view && view->output())
        setOutput(view->output());

    d->grab->motion(d->compositor()->currentTimeMsecs());
}

void QWaylandPointer::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandPointer);
    if (!d->focusResource)
        return;

    uint32_t time = d->compositor()->currentTimeMsecs();
    uint32_t axis = orientation == Qt::Horizontal ? WL_POINTER_AXIS_HORIZONTAL_SCROLL
                                                  : WL_POINTER_AXIS_VERTICAL_SCROLL;
    d->send_axis(d->focusResource, time, axis, wl_fixed_from_int(-delta / 12));
}

QWaylandView *QWaylandPointer::mouseFocus() const
{
    Q_D(const QWaylandPointer);
    return d->seat->mouseFocus();
}

QPointF QWaylandPointer::currentLocalPosition() const
{
    Q_D(const QWaylandPointer);
    return d->localPosition;
}

QPointF QWaylandPointer::currentSpacePosition() const
{
    Q_D(const QWaylandPointer);
    return d->spacePosition;
}

bool QWaylandPointer::isButtonPressed() const
{
    Q_D(const QWaylandPointer);
    return d->buttonCount > 0;
}

void QWaylandPointer::addClient(QWaylandClient *client, uint32_t id)
{
    Q_D(QWaylandPointer);
    d->add(client->client(), id, QtWaylandServer::wl_pointer::interfaceVersion());
}

struct wl_resource *QWaylandPointer::focusResource() const
{
    Q_D(const QWaylandPointer);
    if (!d->focusResource)
        return Q_NULLPTR;

    return d->focusResource;
}

void QWaylandPointer::sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_D(QWaylandPointer);
    uint32_t serial = d->compositor()->nextSerial();
    d->send_button(resource, serial, time, toWaylandButton(button), state);
}

uint32_t QWaylandPointer::toWaylandButton(Qt::MouseButton button)
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

void QWaylandPointer::focusDestroyed(void *data)
{
    Q_D(QWaylandPointer);
    Q_UNUSED(data)
    d->focusDestroyListener.reset();

    d->seat->setMouseFocus(Q_NULLPTR);
    d->focusResource = 0;
    d->buttonCount = 0;
    endGrab();
}

void QWaylandPointer::pointerFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus)
{
    Q_UNUSED(newFocus);
    Q_D(QWaylandPointer);
    d->localPosition = QPointF();
    d->hasSentEnter = false;
    if (d->focusResource && oldFocus) {
        uint32_t serial = d->compositor()->nextSerial();
        d->send_leave(d->focusResource, serial, oldFocus->surfaceResource());
        d->focusDestroyListener.reset();
        d->focusResource = 0;
    }

}

QT_END_NAMESPACE

