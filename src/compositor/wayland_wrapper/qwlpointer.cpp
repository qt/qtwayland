/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include "qwlpointer_p.h"

#include "qwlcompositor_p.h"
#include "qwlinputdevice_p.h"
#include "qwlkeyboard_p.h"
#include "qwlsurface_p.h"
#include "qwaylandcompositor.h"
#include "qwaylandsurfaceview.h"

QT_BEGIN_NAMESPACE

QWaylandPointerPrivate::QWaylandPointerPrivate(QWaylandPointer *pointer, QWaylandInputDevice *seat)
    : QObjectPrivate()
    , wl_pointer()
    , m_seat(seat)
    , m_output()
    , m_defaultGrab(pointer)
    , m_grab(&m_defaultGrab)
    , m_grabButton()
    , m_grabTime()
    , m_grabSerial()
    , m_focusResource()
    , m_buttonCount()
{
}

void QWaylandPointerPrivate::startGrab(QWaylandPointerGrabber *grab)
{
    Q_Q(QWaylandPointer);
    m_grab = grab;
    grab->pointer = q;

    if (m_currentPosition.view())
        grab->focus();
}

void QWaylandPointerPrivate::endGrab()
{
    m_grab = &m_defaultGrab;
    m_grab->focus();
}

bool QWaylandPointerPrivate::buttonPressed() const
{
    return m_buttonCount > 0;
}

QWaylandPointerGrabber *QWaylandPointerPrivate::currentGrab() const
{
    return m_grab;
}

Qt::MouseButton QWaylandPointerPrivate::grabButton() const
{
    return m_grabButton;
}

uint32_t QWaylandPointerPrivate::grabTime() const
{
    return m_grabTime;
}

uint32_t QWaylandPointerPrivate::grabSerial() const
{
    return m_grabSerial;
}

void QWaylandPointerPrivate::pointer_destroy_resource(wl_pointer::Resource *resource)
{
    if (m_focusResource == resource)
        m_focusResource = 0;
}

void QWaylandPointerPrivate::pointer_release(wl_pointer::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandPointerPrivate::sendMousePressEvent(Qt::MouseButton button)
{
    Q_Q(QWaylandPointer);
    uint32_t time = compositor()->currentTimeMsecs();
    if (m_buttonCount == 0) {
        m_grabButton = button;
        m_grabTime = time;
    }
    m_buttonCount++;
    m_grab->button(time, button, WL_POINTER_BUTTON_STATE_PRESSED);

    if (m_buttonCount == 1) {
        m_grabSerial = wl_display_get_serial(compositor()->waylandDisplay());
        q->buttonPressedChanged();
    }
}

void QWaylandPointerPrivate::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_Q(QWaylandPointer);
    uint32_t time = compositor()->currentTimeMsecs();
    m_buttonCount--;
    m_grab->button(time, button, WL_POINTER_BUTTON_STATE_RELEASED);

    if (m_buttonCount == 1)
        m_grabSerial = wl_display_get_serial(compositor()->waylandDisplay());
    if (m_buttonCount == 0)
        q->buttonPressedChanged();
}

void QWaylandPointerPrivate::sendMouseMoveEvent(QWaylandSurfaceView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_Q(QWaylandPointer);
    if (m_focusResource && focusView() != view) {
        uint32_t serial = wl_display_next_serial(compositor()->waylandDisplay());
        send_leave(m_focusResource->handle, serial, focusView()->surface()->handle()->resource()->handle);
        m_focusDestroyListener.reset();
    }

    Resource *resource = view ? resourceMap().value(view->surface()->handle()->resource()->client()) : 0;

    if (resource && (focusView() != view || resource != m_focusResource)) {
        uint32_t serial = wl_display_next_serial(compositor()->waylandDisplay());
        QWaylandKeyboard *keyboard = m_seat->keyboard();
        if (keyboard) {
            keyboard->sendKeyModifiers(view->surface()->client(), serial);
        }
        send_enter(resource->handle, serial, view->surface()->handle()->resource()->handle,
                   wl_fixed_from_double(currentLocalPosition().x()), wl_fixed_from_double(currentLocalPosition().y()));

        m_focusDestroyListener.listenForDestruction(view->surface()->handle()->resource()->handle);
    }

    m_focusResource = resource;
    m_currentPosition.updatePosition(view, localPos);
    m_spacePosition = outputSpacePos;
    if (view && view->output())
        q->setOutput(view->output());

    m_grab->motion(compositor()->currentTimeMsecs());
}

void QWaylandPointerPrivate::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    if (!m_focusResource)
        return;

    uint32_t time = compositor()->currentTimeMsecs();
    uint32_t axis = orientation == Qt::Horizontal ? WL_POINTER_AXIS_HORIZONTAL_SCROLL
                                                  : WL_POINTER_AXIS_VERTICAL_SCROLL;
    send_axis(m_focusResource->handle, time, axis, wl_fixed_from_int(-delta / 12));
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

QT_END_NAMESPACE
