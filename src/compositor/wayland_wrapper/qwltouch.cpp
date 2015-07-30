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

#include "qwltouch_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"
#include "qwaylandview.h"
#include "qwlqttouch_p.h"

QT_BEGIN_NAMESPACE

QWaylandTouchPrivate::QWaylandTouchPrivate(QWaylandTouch *touch, QWaylandInputDevice *seat)
    : wl_touch()
    , m_seat(seat)
    , m_focusResource()
    , m_defaultGrab()
    , m_grab(&m_defaultGrab)
{
    m_grab->touch = touch;
}

void QWaylandTouchPrivate::startGrab(QWaylandTouchGrabber *grab)
{
    Q_Q(QWaylandTouch);
    m_grab = grab;
    grab->touch = q;
}

void QWaylandTouchPrivate::endGrab()
{
    m_grab = &m_defaultGrab;
}

void QWaylandTouchPrivate::resetFocusState()
{
    m_focusDestroyListener.reset();
    m_focusResource = 0;
}

void QWaylandTouchPrivate::touch_destroy_resource(Resource *resource)
{
    if (m_focusResource == resource) {
        resetFocusState();
    }
}

void QWaylandTouchPrivate::touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandTouchPrivate::sendCancel()
{
    if (m_focusResource)
        send_cancel(m_focusResource->handle);
}

void QWaylandTouchPrivate::sendFrame()
{
    if (m_focusResource)
        send_frame(m_focusResource->handle);
}

void QWaylandTouchPrivate::sendTouchPoint(int id, const QPointF &point, Qt::TouchPointState state)
{
    switch (state) {
    case Qt::TouchPointPressed:
        sendDown(id, point);
        break;
    case Qt::TouchPointMoved:
        sendMotion(id, point);
        break;
    case Qt::TouchPointReleased:
        sendUp(id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

void QWaylandTouchPrivate::sendDown(int touch_id, const QPointF &position)
{
    m_grab->down(compositor()->currentTimeMsecs(), touch_id, position);
}

void QWaylandTouchPrivate::sendMotion(int touch_id, const QPointF &position)
{
    m_grab->motion(compositor()->currentTimeMsecs(), touch_id, position);
}

void QWaylandTouchPrivate::sendUp(int touch_id)
{
    m_grab->up(compositor()->currentTimeMsecs(), touch_id);
}

void QWaylandTouchPrivate::sendFullTouchEvent(QTouchEvent *event)
{
    if (event->type() == QEvent::TouchCancel) {
        sendCancel();
        return;
    }

    QtWayland::TouchExtensionGlobal *ext = QtWayland::TouchExtensionGlobal::get(compositor());
    if (ext && ext->postTouchEvent(event, m_seat->mouseFocus()))
        return;

    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    QPointF pos = m_seat->mouseFocus()->requestedPosition();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        QPointF p = tp.pos() - pos;
        sendTouchPoint(tp.id(), p, tp.state());
    }
    sendFrame();
}

QT_END_NAMESPACE
