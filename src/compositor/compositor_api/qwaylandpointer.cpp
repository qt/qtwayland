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
#include "qwlpointer_p.h"
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandCompositor>

QT_BEGIN_NAMESPACE

QWaylandPointerGrabber::~QWaylandPointerGrabber()
{
}

QWaylandPointer::QWaylandPointer(QWaylandInputDevice *seat, QObject *parent)
    : QObject(* new QWaylandPointerPrivate(this, seat), parent)
{
    connect(&d_func()->m_focusDestroyListener, &QWaylandDestroyListener::fired, this, &QWaylandPointer::focusDestroyed);
    connect(seat, &QWaylandInputDevice::mouseFocusChanged, this, &QWaylandPointer::pointerFocusChanged);
}

void QWaylandDefaultPointerGrabber::focus()
{
//    if (buttonPressed())
//        return;
//
//    setFocus(m_currentPosition.view(), m_currentPosition.position());
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
    return d->seat();
}

QWaylandCompositor *QWaylandPointer::compositor() const
{
    Q_D(const QWaylandPointer);
    return d->seat()->compositor();
}

QWaylandOutput *QWaylandPointer::output() const
{
    Q_D(const QWaylandPointer);
    return d->output();
}

void QWaylandPointer::setOutput(QWaylandOutput *output)
{
    Q_D(QWaylandPointer);
    d->setOutput(output);
}

void QWaylandPointer::startGrab(QWaylandPointerGrabber *currentGrab)
{
    Q_D(QWaylandPointer);
    d->startGrab(currentGrab);
}

void QWaylandPointer::endGrab()
{
    Q_D(QWaylandPointer);
    d->endGrab();
}

QWaylandPointerGrabber *QWaylandPointer::currentGrab() const
{
    Q_D(const QWaylandPointer);
    return d->currentGrab();
}

Qt::MouseButton QWaylandPointer::grabButton() const
{
    Q_D(const QWaylandPointer);
    return d->grabButton();
}

uint32_t QWaylandPointer::grabTime() const
{
    Q_D(const QWaylandPointer);
    return d->grabTime();

}

uint32_t QWaylandPointer::grabSerial() const
{
    Q_D(const QWaylandPointer);
    return d->grabSerial();
}

void QWaylandPointer::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandPointer);
    d->sendMousePressEvent(button);
}
void QWaylandPointer::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandPointer);
    d->sendMouseReleaseEvent(button);
}
void QWaylandPointer::sendMouseMoveEvent(QWaylandSurfaceView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(QWaylandPointer);
    d->sendMouseMoveEvent(view, localPos, outputSpacePos);
}

void QWaylandPointer::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandPointer);
    d->sendMouseWheelEvent(orientation, delta);
}

QWaylandSurfaceView *QWaylandPointer::currentView() const
{
    Q_D(const QWaylandPointer);
    return d->focusView();
}

QPointF QWaylandPointer::currentLocalPosition() const
{
    Q_D(const QWaylandPointer);
    return d->currentLocalPosition();
}

QPointF QWaylandPointer::currentSpacePosition() const
{
    Q_D(const QWaylandPointer);
    return d->currentSpacePosition();
}

bool QWaylandPointer::isButtonPressed() const
{
    Q_D(const QWaylandPointer);
    return d->buttonPressed();
}

void QWaylandPointer::addClient(QWaylandClient *client, uint32_t id)
{
    Q_D(QWaylandPointer);
    d->add(client->client(), id, QtWaylandServer::wl_pointer::interfaceVersion());
}

struct wl_resource *QWaylandPointer::focusResource() const
{
    Q_D(const QWaylandPointer);
    if (!d->focusResource())
        return Q_NULLPTR;

    if (!d->focusResource())
        return Q_NULLPTR;

    return d->focusResource()->handle;
}

void QWaylandPointer::sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_D(QWaylandPointer);
    uint32_t serial = compositor()->nextSerial();
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
    d->m_focusDestroyListener.reset();

    inputDevice()->setMouseFocus(Q_NULLPTR);
    d->m_focusResource = 0;
    d->m_buttonCount = 0;
    endGrab();
}

void QWaylandPointer::pointerFocusChanged(QWaylandSurfaceView *newFocus, QWaylandSurfaceView *oldFocus)
{
    Q_UNUSED(newFocus);
    Q_D(QWaylandPointer);
    d->m_localPosition = QPointF();
    d->m_hasSentEnter = false;
    if (d->m_focusResource && oldFocus) {
        uint32_t serial = compositor()->nextSerial();
        d->send_leave(d->m_focusResource->handle, serial, oldFocus->surfaceResource());
        d->m_focusDestroyListener.reset();
        d->m_focusResource = 0;
    }

}

QT_END_NAMESPACE

