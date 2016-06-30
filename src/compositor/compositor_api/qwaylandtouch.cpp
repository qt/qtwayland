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

#include "qwaylandtouch.h"
#include "qwaylandtouch_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandInputDevice>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandClient>

#include <QtWaylandCompositor/private/qwlqttouch_p.h>

QT_BEGIN_NAMESPACE

QWaylandTouchPrivate::QWaylandTouchPrivate(QWaylandTouch *touch, QWaylandInputDevice *seat)
    : wl_touch()
    , seat(seat)
    , focusResource()
{
    Q_UNUSED(touch);
}

void QWaylandTouchPrivate::resetFocusState()
{
    focusDestroyListener.reset();
    focusResource = 0;
}

void QWaylandTouchPrivate::touch_bind_resource(Resource *resource)
{
    focusResource = resource;
}

void QWaylandTouchPrivate::touch_destroy_resource(Resource *resource)
{
    if (focusResource == resource) {
        resetFocusState();
    }
}

void QWaylandTouchPrivate::touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandTouchPrivate::sendDown(uint32_t time, int touch_id, const QPointF &position)
{
    Q_Q(QWaylandTouch);
    if (!focusResource || !q->mouseFocus())
        return;

    uint32_t serial = q->compositor()->nextSerial();

    wl_touch_send_down(focusResource->handle, serial, time, q->mouseFocus()->surfaceResource(), touch_id,
                       wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

void QWaylandTouchPrivate::sendUp(uint32_t time, int touch_id)
{
    if (!focusResource)
        return;

    uint32_t serial = compositor()->nextSerial();

    wl_touch_send_up(focusResource->handle, serial, time, touch_id);
}
void QWaylandTouchPrivate::sendMotion(uint32_t time, int touch_id, const QPointF &position)
{
    if (!focusResource)
        return;

    wl_touch_send_motion(focusResource->handle, time, touch_id,
                         wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

/*!
 * \class QWaylandTouch
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief The QWaylandTouch class provides access to a touch device.
 *
 * This class provides access to the touch device in a QWaylandInputDevice. It corresponds to
 * the Wayland interface wl_touch.
 */

/*!
 * Constructs a QWaylandTouch for the \a inputDevice and with the given \a parent.
 */
QWaylandTouch::QWaylandTouch(QWaylandInputDevice *inputDevice, QObject *parent)
    : QWaylandObject(*new QWaylandTouchPrivate(this, inputDevice), parent)
{
    connect(&d_func()->focusDestroyListener, &QWaylandDestroyListener::fired, this, &QWaylandTouch::focusDestroyed);
}

/*!
 * Returns the input device for this QWaylandTouch.
 */
QWaylandInputDevice *QWaylandTouch::inputDevice() const
{
    Q_D(const QWaylandTouch);
    return d->seat;
}

/*!
 * Returns the compositor for this QWaylandTouch.
 */
QWaylandCompositor *QWaylandTouch::compositor() const
{
    Q_D(const QWaylandTouch);
    return d->compositor();
}

/*!
 * Sends a touch point event for the touch device with the given \a id,
 * \a position, and \a state.
 *
 *
 * \sa mouseFocus()
 */
void QWaylandTouch::sendTouchPointEvent(int id, const QPointF &position, Qt::TouchPointState state)
{
    Q_D(QWaylandTouch);
    uint32_t time = compositor()->currentTimeMsecs();
    switch (state) {
    case Qt::TouchPointPressed:
        d->sendDown(time, id, position);
        break;
    case Qt::TouchPointMoved:
        d->sendMotion(time, id, position);
        break;
    case Qt::TouchPointReleased:
        d->sendUp(time, id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

/*!
 * Sends a touch frame event for the touch device. This indicates the end of a
 * contact point list.
 */
void QWaylandTouch::sendFrameEvent()
{
    Q_D(QWaylandTouch);
    if (d->focusResource)
        d->send_frame(d->focusResource->handle);
}

/*!
 * Sends a touch cancel event for the touch device.
 */
void QWaylandTouch::sendCancelEvent()
{
    Q_D(QWaylandTouch);
    if (d->focusResource)
        d->send_cancel(d->focusResource->handle);
}

/*!
 * Sends all the touch points in \a event for this touch device, followed
 * by a touch frame event.
 *
 * \sa sendTouchPointEvent(), sendFrameEvent()
 */
void QWaylandTouch::sendFullTouchEvent(QTouchEvent *event)
{
    Q_D(QWaylandTouch);
    if (event->type() == QEvent::TouchCancel) {
        sendCancelEvent();
        return;
    }

    QtWayland::TouchExtensionGlobal *ext = QtWayland::TouchExtensionGlobal::findIn(d->compositor());
    if (ext && ext->postTouchEvent(event, d->seat->mouseFocus()))
        return;

    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        sendTouchPointEvent(tp.id(), tp.pos(), tp.state());
    }
    sendFrameEvent();
}

/*!
 * \internal
 */
void QWaylandTouch::addClient(QWaylandClient *client, uint32_t id, uint32_t version)
{
    Q_D(QWaylandTouch);
    d->add(client->client(), id, qMin<uint32_t>(QtWaylandServer::wl_touch::interfaceVersion(), version));
}

/*!
 * Returns the Wayland resource for this QWaylandTouch.
 */
struct wl_resource *QWaylandTouch::focusResource() const
{
    Q_D(const QWaylandTouch);
    if (!d->focusResource)
        return Q_NULLPTR;
    return d->focusResource->handle;
}

/*!
 * Returns the view currently holding mouse focus in the input device.
 */
QWaylandView *QWaylandTouch::mouseFocus() const
{
    Q_D(const QWaylandTouch);
    return d->seat->mouseFocus();
}

/*!
 * \internal
 */
void QWaylandTouch::focusDestroyed(void *data)
{
    Q_UNUSED(data)
    Q_D(QWaylandTouch);
    d->resetFocusState();
}

/*!
 * \internal
 */
void QWaylandTouch::mouseFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus)
{
    Q_UNUSED(newFocus);
    Q_UNUSED(oldFocus);
    Q_D(QWaylandTouch);
    d->resetFocusState();
}

QT_END_NAMESPACE
