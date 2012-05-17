/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDINPUTDEVICE_H
#define QWAYLANDINPUTDEVICE_H

#include "qwaylandwindow.h"

#include <QSocketNotifier>
#include <QObject>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>
#include <QWindowSystemInterface>

#include <wayland-client.h>

#ifndef QT_NO_WAYLAND_XKB
struct xkb_context;
struct xkb_keymap;
struct xkb_state;
#endif

QT_BEGIN_NAMESPACE

class QWaylandWindow;
class QWaylandDisplay;

class QWaylandInputDevice {
public:
    QWaylandInputDevice(QWaylandDisplay *display, uint32_t id);
    ~QWaylandInputDevice();

    uint32_t capabilities() const { return mCaps; }

    struct wl_seat *wl_seat() const { return mSeat; }

    void attach(QWaylandBuffer *buffer, int x, int y);
    void handleWindowDestroyed(QWaylandWindow *window);

    void setTransferDevice(struct wl_data_device *device);
    struct wl_data_device *transferDevice() const;

    void removeMouseButtonFromState(Qt::MouseButton button);

private:
    QWaylandDisplay *mQDisplay;
    struct wl_display *mDisplay;

    struct wl_seat *mSeat;
    uint32_t mCaps;

    struct {
        struct wl_pointer *pointer;
        struct wl_keyboard *keyboard;
        struct wl_touch *touch;
    } mDeviceInterfaces;

    struct wl_data_device *mTransferDevice;
    QWaylandWindow *mPointerFocus;
    QWaylandWindow *mKeyboardFocus;
    QWaylandWindow *mTouchFocus;

    Qt::MouseButtons mButtons;
    QPointF mSurfacePos;
    QPointF mGlobalPos;
    Qt::KeyboardModifiers mModifiers;
    uint32_t mTime;

    static const struct wl_seat_listener seatListener;

    static void seat_capabilities(void *data,
                                  struct wl_seat *seat,
                                  uint32_t caps);

    static const struct wl_pointer_listener pointerListener;

    static void pointer_enter(void *data,
                              struct wl_pointer *pointer,
                              uint32_t time, struct wl_surface *surface,
                              wl_fixed_t sx, wl_fixed_t sy);
    static void pointer_leave(void *data,
                              struct wl_pointer *pointer,
                              uint32_t time, struct wl_surface *surface);
    static void pointer_motion(void *data,
                               struct wl_pointer *pointer,
                               uint32_t time,
                               wl_fixed_t sx, wl_fixed_t sy);
    static void pointer_button(void *data,
                               struct wl_pointer *pointer,
                               uint32_t serial, uint32_t time,
                               uint32_t button, uint32_t state);
    static void pointer_axis(void *data,
                             struct wl_pointer *pointer,
                             uint32_t time,
                             uint32_t axis,
                             int32_t value);

    static const struct wl_keyboard_listener keyboardListener;

    static void keyboard_enter(void *data,
                               struct wl_keyboard *keyboard,
                               uint32_t time,
                               struct wl_surface *surface,
                               struct wl_array *keys);
    static void keyboard_leave(void *data,
                               struct wl_keyboard *keyboard,
                               uint32_t time,
                               struct wl_surface *surface);
    static void keyboard_key(void *data,
                             struct wl_keyboard *keyboard,
                             uint32_t serial, uint32_t time,
                             uint32_t key, uint32_t state);

    static const struct wl_touch_listener touchListener;

    static void touch_down(void *data,
                           struct wl_touch *touch,
                           uint32_t serial,
                           uint32_t time,
                           struct wl_surface *surface,
                           int32_t id,
                           wl_fixed_t x,
                           wl_fixed_t y);
    static void touch_up(void *data,
                         struct wl_touch *touch,
                         uint32_t serial,
                         uint32_t time,
                         int32_t id);
    static void touch_motion(void *data,
                             struct wl_touch *touch,
                             uint32_t time,
                             int32_t id,
                             wl_fixed_t x,
                             wl_fixed_t y);
    static void touch_frame(void *data,
                            struct wl_touch *touch);
    static void touch_cancel(void *data,
                             struct wl_touch *touch);

    void handleTouchPoint(int id, double x, double y, Qt::TouchPointState state);
    void handleTouchFrame();
    QList<QWindowSystemInterface::TouchPoint> mTouchPoints;
    QList<QWindowSystemInterface::TouchPoint> mPrevTouchPoints;
    QTouchDevice *mTouchDevice;

#ifndef QT_NO_WAYLAND_XKB
    xkb_context *mXkbContext;
    xkb_keymap *mXkbMap;
    xkb_state *mXkbState;
#endif

    friend class QWaylandTouchExtension;
    friend class QWaylandQtKeyExtension;
};

QT_END_NAMESPACE

#endif
