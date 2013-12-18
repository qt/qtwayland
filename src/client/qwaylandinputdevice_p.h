/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDINPUTDEVICE_H
#define QWAYLANDINPUTDEVICE_H

#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <QSocketNotifier>
#include <QObject>
#include <QTimer>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface.h>

#include <wayland-client.h>

#include <QtWaylandClient/private/qwayland-wayland.h>

#ifndef QT_NO_WAYLAND_XKB
struct xkb_context;
struct xkb_keymap;
struct xkb_state;
#endif

QT_BEGIN_NAMESPACE

class QWaylandWindow;
class QWaylandDisplay;
class QWaylandDataDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDevice
                            : public QObject
                            , public QtWayland::wl_pointer
                            , public QtWayland::wl_keyboard
                            , public QtWayland::wl_touch
                            , public QtWayland::wl_seat
{
    Q_OBJECT
public:
    QWaylandInputDevice(QWaylandDisplay *display, uint32_t id);
    ~QWaylandInputDevice();

    uint32_t capabilities() const { return mCaps; }

    struct ::wl_seat *wl_seat() { return QtWayland::wl_seat::object(); }

    void setCursor(Qt::CursorShape cursor, QWaylandScreen *screen);
    void setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image);
    void handleWindowDestroyed(QWaylandWindow *window);

    void setDataDevice(QWaylandDataDevice *device);
    QWaylandDataDevice *dataDevice() const;

    void removeMouseButtonFromState(Qt::MouseButton button);

    QWaylandWindow *pointerFocus() const;

    Qt::KeyboardModifiers modifiers() const;

    uint32_t serial() const;
    uint32_t cursorSerial() const { return mCursorSerial; }

private slots:
    void repeatKey();

private:
    QWaylandDisplay *mQDisplay;
    struct wl_display *mDisplay;
    struct wl_callback *mFocusCallback;

    uint32_t mCaps;

    struct wl_surface *pointerSurface;

    QWaylandDataDevice *mDataDevice;
    QWaylandWindow *mPointerFocus;
    QWaylandWindow *mKeyboardFocus;
    QWaylandWindow *mTouchFocus;

    Qt::MouseButtons mButtons;
    QPointF mSurfacePos;
    QPointF mGlobalPos;
    uint32_t mTime;
    uint32_t mSerial;
    uint32_t mEnterSerial;
    uint32_t mCursorSerial;

    void seat_capabilities(uint32_t caps) Q_DECL_OVERRIDE;

    void pointer_enter(uint32_t serial, struct wl_surface *surface,
                       wl_fixed_t sx, wl_fixed_t sy) Q_DECL_OVERRIDE;
    void pointer_leave(uint32_t time, struct wl_surface *surface);
    void pointer_motion(uint32_t time,
                        wl_fixed_t sx, wl_fixed_t sy) Q_DECL_OVERRIDE;
    void pointer_button(uint32_t serial, uint32_t time,
                        uint32_t button, uint32_t state) Q_DECL_OVERRIDE;
    void pointer_axis(uint32_t time,
                      uint32_t axis,
                      wl_fixed_t value) Q_DECL_OVERRIDE;

    void keyboard_keymap(uint32_t format,
                         int32_t fd,
                         uint32_t size) Q_DECL_OVERRIDE;
    void keyboard_enter(uint32_t time,
                        struct wl_surface *surface,
                        struct wl_array *keys) Q_DECL_OVERRIDE;
    void keyboard_leave(uint32_t time,
                        struct wl_surface *surface) Q_DECL_OVERRIDE;
    void keyboard_key(uint32_t serial, uint32_t time,
                      uint32_t key, uint32_t state) Q_DECL_OVERRIDE;
    void keyboard_modifiers(uint32_t serial,
                            uint32_t mods_depressed,
                            uint32_t mods_latched,
                            uint32_t mods_locked,
                            uint32_t group) Q_DECL_OVERRIDE;

    void touch_down(uint32_t serial,
                    uint32_t time,
                    struct wl_surface *surface,
                    int32_t id,
                    wl_fixed_t x,
                    wl_fixed_t y) Q_DECL_OVERRIDE;
    void touch_up(uint32_t serial,
                  uint32_t time,
                  int32_t id) Q_DECL_OVERRIDE;
    void touch_motion(uint32_t time,
                      int32_t id,
                      wl_fixed_t x,
                      wl_fixed_t y) Q_DECL_OVERRIDE;
    void touch_frame() Q_DECL_OVERRIDE;
    void touch_cancel() Q_DECL_OVERRIDE;

    void handleTouchPoint(int id, double x, double y, Qt::TouchPointState state);

    static const wl_callback_listener callback;
    static void focusCallback(void *data, struct wl_callback *callback, uint32_t time);

    QList<QWindowSystemInterface::TouchPoint> mTouchPoints;
    QList<QWindowSystemInterface::TouchPoint> mPrevTouchPoints;
    QTouchDevice *mTouchDevice;

#ifndef QT_NO_WAYLAND_XKB
    xkb_context *mXkbContext;
    xkb_keymap *mXkbMap;
    xkb_state *mXkbState;
#endif

    int mRepeatKey;
    uint32_t mRepeatCode;
    uint32_t mRepeatTime;
    QString mRepeatText;
    QTimer mRepeatTimer;

    friend class QWaylandTouchExtension;
    friend class QWaylandQtKeyExtension;
};

inline uint32_t QWaylandInputDevice::serial() const
{
    return mSerial;
}

QT_END_NAMESPACE

#endif
