// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDTOUCH_H
#define QWAYLANDTOUCH_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpa/qwindowsysteminterface.h>

#include <QtWaylandClient/private/qwayland-touch-extension.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandInputDevice;

class Q_WAYLANDCLIENT_EXPORT QWaylandTouchExtension : public QtWayland::qt_touch_extension
{
public:
    QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id);

    void touchCanceled();

private:
    void registerDevice(int caps);

    QWaylandDisplay *mDisplay = nullptr;

    void touch_extension_touch(uint32_t time,
                               uint32_t id,
                               uint32_t state,
                               int32_t x,
                               int32_t y,
                               int32_t normalized_x,
                               int32_t normalized_y,
                               int32_t width,
                               int32_t height,
                               uint32_t pressure,
                               int32_t velocity_x,
                               int32_t velocity_y,
                               uint32_t flags,
                               struct wl_array *rawdata) override;
    void touch_extension_configure(uint32_t flags) override;

    void sendTouchEvent();

    QList<QWindowSystemInterface::TouchPoint> mTouchPoints;
    QList<QWindowSystemInterface::TouchPoint> mPrevTouchPoints;
    QPointingDevice *mTouchDevice = nullptr;
    uint32_t mTimestamp;
    int mPointsLeft;
    uint32_t mFlags;
    int mMouseSourceId;
    QPointF mLastMouseLocal;
    QPointF mLastMouseGlobal;
    QWindow *mTargetWindow = nullptr;
    QWaylandInputDevice *mInputDevice = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDTOUCH_H
