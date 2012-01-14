/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandtouch.h"
#include "qwaylandinputdevice.h"

#include "wayland-touch-extension-client-protocol.h"

QWaylandTouchExtension::QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id)
{
    mDisplay = display;
    mTouch = static_cast<struct wl_touch_extension *>(wl_display_bind(display->wl_display(), id, &wl_touch_extension_interface));
    wl_touch_extension_add_listener(mTouch, &touch_listener, this);

    QTouchDevice::Capabilities caps = QTouchDevice::Position
            | QTouchDevice::Area
            | QTouchDevice::Pressure
            | QTouchDevice::Velocity
            | QTouchDevice::RawPositions
            | QTouchDevice::NormalizedPosition;
    mTouchDevice = new QTouchDevice;
    mTouchDevice->setType(QTouchDevice::TouchScreen);
    mTouchDevice->setCapabilities(caps);
    QWindowSystemInterface::registerTouchDevice(mTouchDevice);
}

static inline qreal fromFixed(int f)
{
    return f / qreal(10000);
}

void QWaylandTouchExtension::handle_touch(void *data, wl_touch_extension *ext, uint32_t time,
                                          uint32_t id, uint32_t state, int32_t x, int32_t y,
                                          int32_t normalized_x, int32_t normalized_y,
                                          int32_t width, int32_t height, uint32_t pressure,
                                          int32_t velocity_x, int32_t velocity_y,
                                          uint32_t flags, wl_array *rawdata)
{
    Q_UNUSED(ext);
    QWaylandTouchExtension *self = static_cast<QWaylandTouchExtension *>(data);
    QList<QWaylandInputDevice *> inputDevices = self->mDisplay->inputDevices();
    if (inputDevices.isEmpty()) {
        qWarning("wl_touch_extension: handle_touch: No input device");
        return;
    }
    QWaylandInputDevice *dev = inputDevices.first();
    QWaylandWindow *win = dev->mTouchFocus;
    if (!win)
        win = dev->mPointerFocus;
    if (!win)
        win = dev->mKeyboardFocus;
    if (!win || !win->window()) {
        qWarning("wl_touch_extension: handle_touch: No pointer focus");
        return;
    }

    QWindowSystemInterface::TouchPoint tp;
    tp.id = id;
    tp.state = Qt::TouchPointState(int(state));
    tp.flags = QTouchEvent::TouchPoint::InfoFlags(int(flags));

    tp.area = QRectF(0, 0, fromFixed(width), fromFixed(height));
    // Got surface-relative coords but need a (virtual) screen position.
    QPointF relPos = QPointF(fromFixed(x), fromFixed(y));
    QPointF delta = relPos - relPos.toPoint();
    tp.area.moveCenter(win->window()->mapToGlobal(relPos.toPoint()) + delta);

    tp.normalPosition.setX(fromFixed(normalized_x));
    tp.normalPosition.setY(fromFixed(normalized_y));
    tp.pressure = pressure / 255.0;
    tp.velocity.setX(fromFixed(velocity_x));
    tp.velocity.setY(fromFixed(velocity_y));

    self->mTouchPoints.append(tp);
    self->mTimestamp = time;
}

void QWaylandTouchExtension::handle_touch_frame(void *data, wl_touch_extension *ext)
{
    Q_UNUSED(ext);
    QWaylandTouchExtension *self = static_cast<QWaylandTouchExtension *>(data);
    self->sendTouchEvent();
}

void QWaylandTouchExtension::sendTouchEvent()
{
    // Copy all points, that are in the previous but not in the current list, as stationary.
    for (int i = 0; i < mPrevTouchPoints.count(); ++i) {
        const QWindowSystemInterface::TouchPoint &prevPoint(mPrevTouchPoints.at(i));
        if (prevPoint.state == Qt::TouchPointReleased)
            continue;
        bool found = false;
        for (int j = 0; j < mTouchPoints.count(); ++j)
            if (mTouchPoints.at(j).id == prevPoint.id) {
                found = true;
                break;
            }
        if (!found) {
            QWindowSystemInterface::TouchPoint p = prevPoint;
            p.state = Qt::TouchPointStationary;
            mTouchPoints.append(p);
        }
    }

    if (mTouchPoints.isEmpty()) {
        mPrevTouchPoints.clear();
        return;
    }

    QWindowSystemInterface::handleTouchEvent(0, mTimestamp, mTouchDevice, mTouchPoints);

    bool allReleased = true;
    for (int i = 0; i < mTouchPoints.count(); ++i)
        if (mTouchPoints.at(i).state != Qt::TouchPointReleased) {
            allReleased = false;
            break;
        }

    mPrevTouchPoints = mTouchPoints;
    mTouchPoints.clear();

    if (allReleased)
        mPrevTouchPoints.clear();
}

const struct wl_touch_extension_listener QWaylandTouchExtension::touch_listener = {
    QWaylandTouchExtension::handle_touch,
    QWaylandTouchExtension::handle_touch_frame
};
