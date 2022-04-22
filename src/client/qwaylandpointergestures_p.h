/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDPOINTERGESTURES_P_H
#define QWAYLANDPOINTERGESTURES_P_H

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

#include <QtWaylandClient/private/qwayland-pointer-gestures-unstable-v1.h>

#include <QtWaylandClient/private/qtwaylandclientglobal_p.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandPointerGestureSwipe;
class QWaylandPointerGesturePinch;

class Q_WAYLANDCLIENT_EXPORT QWaylandPointerGestures : public QtWayland::zwp_pointer_gestures_v1
{
public:
    explicit QWaylandPointerGestures(QWaylandDisplay *display, uint id, uint version);

    QWaylandPointerGestureSwipe *createPointerGestureSwipe(QWaylandInputDevice *device);
    QWaylandPointerGesturePinch *createPointerGesturePinch(QWaylandInputDevice *device);
};

class Q_WAYLANDCLIENT_EXPORT QWaylandPointerGestureSwipe :
        public QtWayland::zwp_pointer_gesture_swipe_v1
{
public:
    QWaylandPointerGestureSwipe(QWaylandInputDevice *p);
    ~QWaylandPointerGestureSwipe() override;

    void zwp_pointer_gesture_swipe_v1_begin(uint32_t serial,
                                            uint32_t time,
                                            struct ::wl_surface *surface,
                                            uint32_t fingers) override;

    void zwp_pointer_gesture_swipe_v1_update(uint32_t time,
                                             wl_fixed_t dx,
                                             wl_fixed_t dy) override;

    void zwp_pointer_gesture_swipe_v1_end(uint32_t serial,
                                          uint32_t time,
                                          int32_t cancelled) override;

    struct ::zwp_pointer_gesture_swipe_v1 *zwp_pointer_gesture_swipe_v1()
    {
        return QtWayland::zwp_pointer_gesture_swipe_v1::object();
    }

    QWaylandInputDevice *mParent = nullptr;
    QPointer<QWaylandWindow> mFocus;
    uint mFingers = 0;
};

class Q_WAYLANDCLIENT_EXPORT QWaylandPointerGesturePinch :
        public QtWayland::zwp_pointer_gesture_pinch_v1
{
public:
    QWaylandPointerGesturePinch(QWaylandInputDevice *p);
    ~QWaylandPointerGesturePinch() override;

    void zwp_pointer_gesture_pinch_v1_begin(uint32_t serial,
                                            uint32_t time,
                                            struct ::wl_surface *surface,
                                            uint32_t fingers) override;

    void zwp_pointer_gesture_pinch_v1_update(uint32_t time,
                                             wl_fixed_t dx,
                                             wl_fixed_t dy,
                                             wl_fixed_t scale,
                                             wl_fixed_t rotation) override;

    void zwp_pointer_gesture_pinch_v1_end(uint32_t serial,
                                          uint32_t time,
                                          int32_t cancelled) override;

    struct ::zwp_pointer_gesture_pinch_v1 *zwp_pointer_gesture_pinch_v1()
    {
        return QtWayland::zwp_pointer_gesture_pinch_v1::object();
    }

    QWaylandInputDevice *mParent = nullptr;
    QPointer<QWaylandWindow> mFocus;
    uint mFingers = 0;

    // We need to convert between absolute scale provided by wayland/libinput and zoom deltas
    // that Qt expects. This stores the scale of the last pinch event or 1.0 if there was none.
    qreal mLastScale = 1;
};

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDPOINTERGESTURES_P_H
