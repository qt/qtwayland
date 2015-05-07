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

#include "qwaylandtouch.h"

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QWaylandSurfaceView>
#include <QtCompositor/QWaylandClient>

#include "qwltouch_p.h"

QT_BEGIN_NAMESPACE

QWaylandTouchGrabber::QWaylandTouchGrabber()
{
}

QWaylandTouchGrabber::~QWaylandTouchGrabber()
{
}

QWaylandDefaultTouchGrabber::QWaylandDefaultTouchGrabber()
    : QWaylandTouchGrabber()
{
}

void QWaylandDefaultTouchGrabber::down(uint32_t time, int touch_id, const QPointF &position)
{
    if (!touch->focusResource() || !touch->mouseFocus())
        return;

    uint32_t serial = wl_display_next_serial(touch->compositor()->waylandDisplay());

    wl_touch_send_down(touch->focusResource(), serial, time, touch->mouseFocus()->surfaceResource(), touch_id,
                       wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

void QWaylandDefaultTouchGrabber::up(uint32_t time, int touch_id)
{
    if (!touch->focusResource())
        return;

    uint32_t serial = touch->compositor()->nextSerial();

    wl_touch_send_up(touch->focusResource(), serial, time, touch_id);
}
void QWaylandDefaultTouchGrabber::motion(uint32_t time, int touch_id, const QPointF &position)
{
    if (!touch->focusResource())
        return;

    wl_touch_send_motion(touch->focusResource(), time, touch_id,
                         wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

QWaylandTouch::QWaylandTouch(QWaylandInputDevice *seat, QObject *parent)
    : QObject(*new QWaylandTouchPrivate(this, seat), parent)
{
    connect(&d_func()->m_focusDestroyListener, &QWaylandDestroyListener::fired, this, &QWaylandTouch::focusDestroyed);
}

QWaylandInputDevice *QWaylandTouch::inputDevice() const
{
    Q_D(const QWaylandTouch);
    return d->m_seat;
}

QWaylandCompositor *QWaylandTouch::compositor() const
{
    Q_D(const QWaylandTouch);
    return d->compositor();
}


void QWaylandTouch::startGrab(QWaylandTouchGrabber *grab)
{
    Q_D(QWaylandTouch);
    d->startGrab(grab);
}

void QWaylandTouch::endGrab()
{
    Q_D(QWaylandTouch);
    d->endGrab();
}

void QWaylandTouch::sendTouchPointEvent(int id, const QPointF &position, Qt::TouchPointState state)
{
    Q_D(QWaylandTouch);
    d->sendTouchPoint(id, position, state);
}

void QWaylandTouch::sendFrameEvent()
{
    Q_D(QWaylandTouch);
    d->sendFrame();
}

void QWaylandTouch::sendCancelEvent()
{
    Q_D(QWaylandTouch);
    d->sendCancel();
}

void QWaylandTouch::sendFullTouchEvent(QTouchEvent *event)
{
    Q_D(QWaylandTouch);
    d->sendFullTouchEvent(event);
}

void QWaylandTouch::addClient(QWaylandClient *client, uint32_t id)
{
    Q_D(QWaylandTouch);
    d->add(client->client(), id);
}

struct wl_resource *QWaylandTouch::focusResource() const
{
    Q_D(const QWaylandTouch);
    if (!d->focusResource())
        return Q_NULLPTR;
    return d->focusResource()->handle;
}

QWaylandSurfaceView *QWaylandTouch::mouseFocus() const
{
    Q_D(const QWaylandTouch);
    return d->m_seat->mouseFocus();
}

void QWaylandTouch::focusDestroyed(void *data)
{
    Q_UNUSED(data)
    Q_D(QWaylandTouch);
    d->resetFocusState();
}

void QWaylandTouch::mouseFocusChanged(QWaylandSurfaceView *newFocus, QWaylandSurfaceView *oldFocus)
{
    Q_UNUSED(newFocus);
    Q_UNUSED(oldFocus);
    Q_D(QWaylandTouch);
    d->resetFocusState();
}

QT_END_NAMESPACE
