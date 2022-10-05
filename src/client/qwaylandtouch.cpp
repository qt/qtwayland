// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandtouch_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandsurface_p.h"

#include <QtGui/QPointingDevice>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandTouchExtension::QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id)
    : QtWayland::qt_touch_extension(display->wl_registry(), id, 1),
      mDisplay(display),
      mTouchDevice(nullptr),
      mPointsLeft(0),
      mFlags(0),
      mMouseSourceId(-1),
      mInputDevice(nullptr)
{
}

void QWaylandTouchExtension::registerDevice(int caps)
{
    // TODO number of touchpoints, actual name and ID
    mTouchDevice = new QPointingDevice(QLatin1String("some touchscreen"), 0,
                                           QInputDevice::DeviceType::TouchScreen, QPointingDevice::PointerType::Finger,
                                           QInputDevice::Capabilities(caps), 10, 0);
    QWindowSystemInterface::registerInputDevice(mTouchDevice);
}

static inline qreal fromFixed(int f)
{
    return f / qreal(10000);
}

void QWaylandTouchExtension::touch_extension_touch(uint32_t time,
                                                   uint32_t id, uint32_t state, int32_t x, int32_t y,
                                                   int32_t normalized_x, int32_t normalized_y,
                                                   int32_t width, int32_t height, uint32_t pressure,
                                                   int32_t velocity_x, int32_t velocity_y,
                                                   uint32_t flags, wl_array *rawdata)
{
    if (!mInputDevice) {
        QList<QWaylandInputDevice *> inputDevices = mDisplay->inputDevices();
        if (inputDevices.isEmpty()) {
            qWarning("qt_touch_extension: handle_touch: No input devices");
            return;
        }
        mInputDevice = inputDevices.first();
    }
    QWaylandWindow *win = mInputDevice->touchFocus();
    if (!win)
        win = mInputDevice->pointerFocus();
    if (!win)
        win = mInputDevice->keyboardFocus();
    if (!win || !win->window()) {
        qWarning("qt_touch_extension: handle_touch: No pointer focus");
        return;
    }
    mTargetWindow = win->window();

    QWindowSystemInterface::TouchPoint tp;
    tp.id = id;
    tp.state = QEventPoint::State(int(state & 0xFFFF));
    int sentPointCount = state >> 16;
    if (!mPointsLeft) {
        Q_ASSERT(sentPointCount > 0);
        mPointsLeft = sentPointCount;
    }

    if (!mTouchDevice)
        registerDevice(flags >> 16);

    tp.area = QRectF(0, 0, fromFixed(width), fromFixed(height));
    // Got surface-relative coords but need a (virtual) screen position.
    QPointF relPos = QPointF(fromFixed(x), fromFixed(y));
    QPointF delta = relPos - relPos.toPoint();
    tp.area.moveCenter(mTargetWindow->mapToGlobal(relPos.toPoint()) + delta);

    tp.normalPosition.setX(fromFixed(normalized_x));
    tp.normalPosition.setY(fromFixed(normalized_y));
    tp.pressure = pressure / 255.0;
    tp.velocity.setX(fromFixed(velocity_x));
    tp.velocity.setY(fromFixed(velocity_y));

    if (rawdata) {
        const int rawPosCount = rawdata->size / sizeof(float) / 2;
        float *p = static_cast<float *>(rawdata->data);
        for (int i = 0; i < rawPosCount; ++i) {
            float x = *p++;
            float y = *p++;
            tp.rawPositions.append(QPointF(x, y));
        }
    }

    mTouchPoints.append(tp);
    mTimestamp = time;

    if (!--mPointsLeft)
        sendTouchEvent();
}

void QWaylandTouchExtension::sendTouchEvent()
{
    // Copy all points, that are in the previous but not in the current list, as stationary.
    for (int i = 0; i < mPrevTouchPoints.size(); ++i) {
        const QWindowSystemInterface::TouchPoint &prevPoint(mPrevTouchPoints.at(i));
        if (prevPoint.state == QEventPoint::Released)
            continue;
        bool found = false;
        for (int j = 0; j < mTouchPoints.size(); ++j)
            if (mTouchPoints.at(j).id == prevPoint.id) {
                found = true;
                break;
            }
        if (!found) {
            QWindowSystemInterface::TouchPoint p = prevPoint;
            p.state = QEventPoint::Stationary;
            mTouchPoints.append(p);
        }
    }

    if (mTouchPoints.isEmpty()) {
        mPrevTouchPoints.clear();
        return;
    }

    QWindowSystemInterface::handleTouchEvent(mTargetWindow, mTimestamp, mTouchDevice, mTouchPoints);

    QEventPoint::States states = {};
    for (int i = 0; i < mTouchPoints.size(); ++i)
        states |= mTouchPoints.at(i).state;

    if (mFlags & QT_TOUCH_EXTENSION_FLAGS_MOUSE_FROM_TOUCH) {
        const bool firstPress = states == QEventPoint::Pressed;
        if (firstPress)
            mMouseSourceId = mTouchPoints.first().id;
        for (int i = 0; i < mTouchPoints.size(); ++i) {
            const QWindowSystemInterface::TouchPoint &tp(mTouchPoints.at(i));
            if (tp.id == mMouseSourceId) {
                const bool released = tp.state == QEventPoint::Released;
                Qt::MouseButtons buttons = released ? Qt::NoButton : Qt::LeftButton;
                QEvent::Type eventType = firstPress ? QEvent::MouseButtonPress
                                                    : released ? QEvent::MouseButtonRelease
                                                               : QEvent::MouseMove;
                mLastMouseGlobal = tp.area.center();
                QPoint globalPoint = mLastMouseGlobal.toPoint();
                QPointF delta = mLastMouseGlobal - globalPoint;
                mLastMouseLocal = mTargetWindow->mapFromGlobal(globalPoint) + delta;
                QWindowSystemInterface::handleMouseEvent(mTargetWindow, mTimestamp, mLastMouseLocal, mLastMouseGlobal,
                                                         buttons, Qt::LeftButton, eventType);
                if (buttons == Qt::NoButton)
                    mMouseSourceId = -1;
                break;
            }
        }
    }

    mPrevTouchPoints = mTouchPoints;
    mTouchPoints.clear();

    if (states == QEventPoint::Released)
        mPrevTouchPoints.clear();
}

void QWaylandTouchExtension::touchCanceled()
{
    mTouchPoints.clear();
    mPrevTouchPoints.clear();
    if (mMouseSourceId != -1)
        QWindowSystemInterface::handleMouseEvent(mTargetWindow, mTimestamp, mLastMouseLocal, mLastMouseGlobal, Qt::NoButton, Qt::LeftButton, QEvent::MouseButtonRelease);
}

void QWaylandTouchExtension::touch_extension_configure(uint32_t flags)
{
    mFlags = flags;
}

}

QT_END_NAMESPACE
