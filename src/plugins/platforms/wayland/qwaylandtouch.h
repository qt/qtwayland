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

#ifndef QWAYLANDTOUCh_H
#define QWAYLANDTOUCH_H

#include "qwaylanddisplay.h"
#include <QWindowSystemInterface>

class wl_touch_extension;

class QWaylandTouchExtension
{
public:
    QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id);

private:
    QWaylandDisplay *mDisplay;
    wl_touch_extension *mTouch;
    static const struct wl_touch_extension_listener touch_listener;

    static void handle_touch(void *data,
                             struct wl_touch_extension *ext,
                             uint32_t time,
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
                             struct wl_array *rawdata);

    void sendTouchEvent();

    QList<QWindowSystemInterface::TouchPoint> mTouchPoints;
    QList<QWindowSystemInterface::TouchPoint> mPrevTouchPoints;
    QTouchDevice *mTouchDevice;
    uint32_t mTimestamp;
    int mPointsLeft;
};

#endif // QWAYLANDTOUCH_H
