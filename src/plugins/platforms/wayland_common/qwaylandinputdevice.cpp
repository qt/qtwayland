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

#include "qwaylandinputdevice.h"

#include "qwaylandintegration.h"
#include "qwaylandwindow.h"
#include "qwaylandbuffer.h"
#include "qwaylanddatadevicemanager.h"
#include "qwaylandtouch.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <qpa/qplatformwindow.h>
#include <QDebug>

#include <unistd.h>
#include <fcntl.h>

#include <wayland-cursor.h>

#include <QtGui/QGuiApplication>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#include <X11/keysym.h>
#endif

QT_USE_NAMESPACE

QWaylandInputDevice::QWaylandInputDevice(QWaylandDisplay *display, uint32_t id)
    : mQDisplay(display)
    , mDisplay(display->wl_display())
    , mCaps(0)
    , mTransferDevice(0)
    , mPointerFocus(0)
    , mKeyboardFocus(0)
    , mTouchFocus(0)
    , mButtons(0)
    , mTouchDevice(0)
    #ifndef QT_NO_WAYLAND_XKB
    , mXkbContext(0)
    , mXkbMap(0)
    , mXkbState(0)
    #endif
{
    mSeat = static_cast<struct wl_seat *>(wl_registry_bind(display->wl_registry(), id, &wl_seat_interface, 1));
    wl_seat_add_listener(mSeat, &seatListener, this);
    wl_seat_set_user_data(mSeat, this);

#ifndef QT_NO_WAYLAND_XKB
    xkb_rule_names names;
    names.rules = strdup("evdev");
    names.model = strdup("pc105");
    names.layout = strdup("us");
    names.variant = strdup("");
    names.options = strdup("");

    xkb_context *mXkbContext = xkb_context_new(xkb_context_flags(0));
    if (mXkbContext) {
        mXkbMap = xkb_map_new_from_names(mXkbContext, &names, xkb_map_compile_flags(0));
        if (mXkbMap) {
            mXkbState = xkb_state_new(mXkbMap);
        }
    }

    if (!mXkbContext || !mXkbMap || !mXkbState)
        qWarning() << "xkb_map_new_from_names failed, no key input";
#endif

    if (mQDisplay->dndSelectionHandler()) {
        mTransferDevice = mQDisplay->dndSelectionHandler()->getDataDevice(this);
    }
}

QWaylandInputDevice::~QWaylandInputDevice()
{
#ifndef QT_NO_WAYLAND_XKB
    if (mXkbState)
        xkb_state_unref(mXkbState);
    if (mXkbMap)
        xkb_map_unref(mXkbMap);
    if (mXkbContext)
        xkb_context_unref(mXkbContext);
#endif
}

const struct wl_seat_listener QWaylandInputDevice::seatListener = {
    QWaylandInputDevice::seat_capabilities
};

const struct wl_pointer_listener QWaylandInputDevice::pointerListener = {
    QWaylandInputDevice::pointer_enter,
    QWaylandInputDevice::pointer_leave,
    QWaylandInputDevice::pointer_motion,
    QWaylandInputDevice::pointer_button,
    QWaylandInputDevice::pointer_axis
};

const struct wl_keyboard_listener QWaylandInputDevice::keyboardListener = {
    QWaylandInputDevice::keyboard_keymap,
    QWaylandInputDevice::keyboard_enter,
    QWaylandInputDevice::keyboard_leave,
    QWaylandInputDevice::keyboard_key,
    QWaylandInputDevice::keyboard_modifiers
};

const struct wl_touch_listener QWaylandInputDevice::touchListener = {
    QWaylandInputDevice::touch_down,
    QWaylandInputDevice::touch_up,
    QWaylandInputDevice::touch_motion,
    QWaylandInputDevice::touch_frame,
    QWaylandInputDevice::touch_cancel
};

void QWaylandInputDevice::seat_capabilities(void *data, struct wl_seat *seat, uint32_t caps)
{
    QWaylandInputDevice *self = static_cast<QWaylandInputDevice *>(data);
    self->mCaps = caps;

    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        self->mDeviceInterfaces.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(self->mDeviceInterfaces.keyboard, &keyboardListener, self);
    }

    if (caps & WL_SEAT_CAPABILITY_POINTER) {
        self->mDeviceInterfaces.pointer = wl_seat_get_pointer(seat);
        self->mDeviceInterfaces.pointerSurface = self->mQDisplay->createSurface(self);
        wl_pointer_add_listener(self->mDeviceInterfaces.pointer, &pointerListener, self);
    }

    if (caps & WL_SEAT_CAPABILITY_TOUCH) {
        self->mDeviceInterfaces.touch = wl_seat_get_touch(seat);
        wl_touch_add_listener(self->mDeviceInterfaces.touch, &touchListener, self);

        if (!self->mTouchDevice) {
            self->mTouchDevice = new QTouchDevice;
            self->mTouchDevice->setType(QTouchDevice::TouchScreen);
            self->mTouchDevice->setCapabilities(QTouchDevice::Position);
            QWindowSystemInterface::registerTouchDevice(self->mTouchDevice);
        }
    }
}

void QWaylandInputDevice::handleWindowDestroyed(QWaylandWindow *window)
{
    if (window == mPointerFocus)
        mPointerFocus = 0;
    if (window == mKeyboardFocus)
        mKeyboardFocus = 0;
}

void QWaylandInputDevice::setTransferDevice(struct wl_data_device *device)
{
    mTransferDevice =  device;
}

struct wl_data_device *QWaylandInputDevice::transferDevice() const
{
    Q_ASSERT(mTransferDevice);
    return mTransferDevice;
}

void QWaylandInputDevice::removeMouseButtonFromState(Qt::MouseButton button)
{
    mButtons = mButtons & !button;
}

void QWaylandInputDevice::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image)
{
    if (mCaps & WL_SEAT_CAPABILITY_POINTER) {
        wl_pointer_set_cursor(mDeviceInterfaces.pointer, mEnterSerial, mDeviceInterfaces.pointerSurface,
                              image->hotspot_x, image->hotspot_y);
        wl_surface_attach(mDeviceInterfaces.pointerSurface, buffer, 0, 0);
        wl_surface_damage(mDeviceInterfaces.pointerSurface, 0, 0, image->width, image->height);
        wl_surface_commit(mDeviceInterfaces.pointerSurface);
    }
}

void QWaylandInputDevice::pointer_enter(void *data,
                                        struct wl_pointer *pointer,
                                        uint32_t serial, struct wl_surface *surface,
                                        wl_fixed_t sx, wl_fixed_t sy)
{
    Q_UNUSED(pointer);
    Q_UNUSED(sx);
    Q_UNUSED(sy);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;

    if (!surface)
        return;

    QGuiApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    QWaylandWindow *window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    window->handleMouseEnter();
    window->handleMouse(inputDevice,
                        inputDevice->mTime,
                        inputDevice->mSurfacePos,
                        inputDevice->mGlobalPos,
                        inputDevice->mButtons,
                        Qt::NoModifier);
    inputDevice->mPointerFocus = window;

    inputDevice->mTime = QWaylandDisplay::currentTimeMillisec();
    inputDevice->mSerial = serial;
    inputDevice->mEnterSerial = serial;
}

void QWaylandInputDevice::pointer_leave(void *data,
                                        struct wl_pointer *pointer,
                                        uint32_t time, struct wl_surface *surface)
{
    Q_UNUSED(pointer);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;

    // The event may arrive after destroying the window, indicated by
    // a null surface.
    if (!surface)
        return;

    QGuiApplication::restoreOverrideCursor();

    QWaylandWindow *window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    window->handleMouseLeave();
    inputDevice->mPointerFocus = 0;
    inputDevice->mButtons = Qt::NoButton;

    inputDevice->mTime = time;
}

void QWaylandInputDevice::pointer_motion(void *data,
                                         struct wl_pointer *pointer,
                                         uint32_t time,
                                         wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    Q_UNUSED(pointer);
    Q_UNUSED(surface_x);
    Q_UNUSED(surface_y);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mPointerFocus;

    if (window == NULL) {
        // We destroyed the pointer focus surface, but the server
        // didn't get the message yet.
        return;
    }

    QPointF pos(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
    QPointF delta = pos - pos.toPoint();
    QPointF global = window->window()->mapToGlobal(pos.toPoint());
    global += delta;

    inputDevice->mSurfacePos = pos;
    inputDevice->mGlobalPos = global;
    inputDevice->mTime = time;

    window->handleMouse(inputDevice,
                        time,
                        inputDevice->mSurfacePos,
                        inputDevice->mGlobalPos,
                        inputDevice->mButtons,
                        Qt::NoModifier);
}

void QWaylandInputDevice::pointer_button(void *data,
                                         struct wl_pointer *pointer,
                                         uint32_t serial, uint32_t time,
                                         uint32_t button, uint32_t state)
{
    Q_UNUSED(pointer);
    Q_UNUSED(serial);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mPointerFocus;
    Qt::MouseButton qt_button;

    // translate from kernel (input.h) 'button' to corresponding Qt:MouseButton.
    // The range of mouse values is 0x110 <= mouse_button < 0x120, the first Joystick button.
    switch (button) {
    case 0x110: qt_button = Qt::LeftButton; break;    // kernel BTN_LEFT
    case 0x111: qt_button = Qt::RightButton; break;
    case 0x112: qt_button = Qt::MiddleButton; break;
    case 0x113: qt_button = Qt::ExtraButton1; break;  // AKA Qt::BackButton
    case 0x114: qt_button = Qt::ExtraButton2; break;  // AKA Qt::ForwardButton
    case 0x115: qt_button = Qt::ExtraButton3; break;  // AKA Qt::TaskButton
    case 0x116: qt_button = Qt::ExtraButton4; break;
    case 0x117: qt_button = Qt::ExtraButton5; break;
    case 0x118: qt_button = Qt::ExtraButton6; break;
    case 0x119: qt_button = Qt::ExtraButton7; break;
    case 0x11a: qt_button = Qt::ExtraButton8; break;
    case 0x11b: qt_button = Qt::ExtraButton9; break;
    case 0x11c: qt_button = Qt::ExtraButton10; break;
    case 0x11d: qt_button = Qt::ExtraButton11; break;
    case 0x11e: qt_button = Qt::ExtraButton12; break;
    case 0x11f: qt_button = Qt::ExtraButton13; break;
    default: return; // invalid button number (as far as Qt is concerned)
    }

    if (state)
        inputDevice->mButtons |= qt_button;
    else
        inputDevice->mButtons &= ~qt_button;

    inputDevice->mTime = time;
    inputDevice->mSerial = serial;

    if (window) {
        window->handleMouse(inputDevice,
                            time,
                            inputDevice->mSurfacePos,
                            inputDevice->mGlobalPos,
                            inputDevice->mButtons,
                            Qt::NoModifier);
    }
}

void QWaylandInputDevice::pointer_axis(void *data,
                                       struct wl_pointer *pointer,
                                       uint32_t time,
                                       uint32_t axis,
                                       int32_t value)
{
    Q_UNUSED(pointer);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mPointerFocus;
    QPoint pixelDelta;
    QPoint angleDelta;

    //normalize value and inverse axis
    int valueDelta = wl_fixed_to_int(value) * -12;

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        pixelDelta = QPoint();
        angleDelta.setX(valueDelta);
    } else {
        pixelDelta = QPoint();
        angleDelta.setY(valueDelta);
    }

    QWindowSystemInterface::handleWheelEvent(window->window(),
                                             time,
                                             inputDevice->mSurfacePos,
                                             inputDevice->mGlobalPos,
                                             pixelDelta,
                                             angleDelta);
}

#ifndef QT_NO_WAYLAND_XKB

static Qt::KeyboardModifiers translateModifiers(xkb_state *state)
{
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    xkb_state_component cstate = xkb_state_component(XKB_STATE_DEPRESSED | XKB_STATE_LATCHED);

    if (xkb_state_mod_name_is_active(state, "Shift", cstate))
        ret |= Qt::ShiftModifier;
    if (xkb_state_mod_name_is_active(state, "Control", cstate))
        ret |= Qt::ControlModifier;
    if (xkb_state_mod_name_is_active(state, "Alt", cstate))
        ret |= Qt::AltModifier;
    if (xkb_state_mod_name_is_active(state, "Mod1", cstate))
        ret |= Qt::AltModifier;
    if (xkb_state_mod_name_is_active(state, "Mod4", cstate))
        ret |= Qt::MetaModifier;

    return ret;
}

static const uint32_t KeyTbl[] = {
    XK_Escape,                  Qt::Key_Escape,
    XK_Tab,                     Qt::Key_Tab,
    XK_ISO_Left_Tab,            Qt::Key_Backtab,
    XK_BackSpace,               Qt::Key_Backspace,
    XK_Return,                  Qt::Key_Return,
    XK_Insert,                  Qt::Key_Insert,
    XK_Delete,                  Qt::Key_Delete,
    XK_Clear,                   Qt::Key_Delete,
    XK_Pause,                   Qt::Key_Pause,
    XK_Print,                   Qt::Key_Print,

    XK_Home,                    Qt::Key_Home,
    XK_End,                     Qt::Key_End,
    XK_Left,                    Qt::Key_Left,
    XK_Up,                      Qt::Key_Up,
    XK_Right,                   Qt::Key_Right,
    XK_Down,                    Qt::Key_Down,
    XK_Prior,                   Qt::Key_PageUp,
    XK_Next,                    Qt::Key_PageDown,

    XK_Shift_L,                 Qt::Key_Shift,
    XK_Shift_R,                 Qt::Key_Shift,
    XK_Shift_Lock,              Qt::Key_Shift,
    XK_Control_L,               Qt::Key_Control,
    XK_Control_R,               Qt::Key_Control,
    XK_Meta_L,                  Qt::Key_Meta,
    XK_Meta_R,                  Qt::Key_Meta,
    XK_Alt_L,                   Qt::Key_Alt,
    XK_Alt_R,                   Qt::Key_Alt,
    XK_Caps_Lock,               Qt::Key_CapsLock,
    XK_Num_Lock,                Qt::Key_NumLock,
    XK_Scroll_Lock,             Qt::Key_ScrollLock,
    XK_Super_L,                 Qt::Key_Super_L,
    XK_Super_R,                 Qt::Key_Super_R,
    XK_Menu,                    Qt::Key_Menu,
    XK_Hyper_L,                 Qt::Key_Hyper_L,
    XK_Hyper_R,                 Qt::Key_Hyper_R,
    XK_Help,                    Qt::Key_Help,

    XK_KP_Space,                Qt::Key_Space,
    XK_KP_Tab,                  Qt::Key_Tab,
    XK_KP_Enter,                Qt::Key_Enter,
    XK_KP_Home,                 Qt::Key_Home,
    XK_KP_Left,                 Qt::Key_Left,
    XK_KP_Up,                   Qt::Key_Up,
    XK_KP_Right,                Qt::Key_Right,
    XK_KP_Down,                 Qt::Key_Down,
    XK_KP_Prior,                Qt::Key_PageUp,
    XK_KP_Next,                 Qt::Key_PageDown,
    XK_KP_End,                  Qt::Key_End,
    XK_KP_Begin,                Qt::Key_Clear,
    XK_KP_Insert,               Qt::Key_Insert,
    XK_KP_Delete,               Qt::Key_Delete,
    XK_KP_Equal,                Qt::Key_Equal,
    XK_KP_Multiply,             Qt::Key_Asterisk,
    XK_KP_Add,                  Qt::Key_Plus,
    XK_KP_Separator,            Qt::Key_Comma,
    XK_KP_Subtract,             Qt::Key_Minus,
    XK_KP_Decimal,              Qt::Key_Period,
    XK_KP_Divide,               Qt::Key_Slash,

    XK_ISO_Level3_Shift,        Qt::Key_AltGr,
    XK_Multi_key,               Qt::Key_Multi_key,
    XK_Codeinput,               Qt::Key_Codeinput,
    XK_SingleCandidate,         Qt::Key_SingleCandidate,
    XK_MultipleCandidate,       Qt::Key_MultipleCandidate,
    XK_PreviousCandidate,       Qt::Key_PreviousCandidate,

    XK_Mode_switch,             Qt::Key_Mode_switch,
    XK_script_switch,           Qt::Key_Mode_switch,

    0,                          0
};

static uint32_t translateKey(uint32_t sym, char *string, size_t size)
{
    Q_UNUSED(size);
    string[0] = '\0';

    if (sym >= XK_F1 && sym <= XK_F35)
        return Qt::Key_F1 + (int(sym) - XK_F1);

    for (int i = 0; KeyTbl[i]; i += 2)
        if (sym == KeyTbl[i])
            return KeyTbl[i + 1];

    string[0] = sym;
    string[1] = '\0';
    return toupper(sym);
}

#endif // QT_NO_WAYLAND_XKB

void QWaylandInputDevice::keyboard_keymap(void *data,
                                          struct wl_keyboard *keyboard,
                                          uint32_t format,
                                          int32_t fd,
                                          uint32_t size)
{
    Q_UNUSED(data);
    Q_UNUSED(keyboard);
    Q_UNUSED(format);
    Q_UNUSED(fd);
    Q_UNUSED(size);
}

void QWaylandInputDevice::keyboard_enter(void *data,
                                         struct wl_keyboard *keyboard,
                                         uint32_t time,
                                         struct wl_surface *surface,
                                         struct wl_array *keys)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(time);
    Q_UNUSED(keys);

    if (!surface)
        return;

    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    inputDevice->mKeyboardFocus = window;
    inputDevice->mQDisplay->setLastKeyboardFocusInputDevice(inputDevice);
    QWindowSystemInterface::handleWindowActivated(window->window());
}

void QWaylandInputDevice::keyboard_leave(void *data,
                                         struct wl_keyboard *keyboard,
                                         uint32_t time,
                                         struct wl_surface *surface)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(time);
    Q_UNUSED(surface);

    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->mKeyboardFocus = NULL;
    inputDevice->mQDisplay->setLastKeyboardFocusInputDevice(0);
}

void QWaylandInputDevice::keyboard_key(void *data,
                                       struct wl_keyboard *keyboard,
                                       uint32_t serial, uint32_t time,
                                       uint32_t key, uint32_t state)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(serial);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mKeyboardFocus;
#ifndef QT_NO_WAYLAND_XKB
    if (!inputDevice->mXkbMap)
        return;

    uint32_t code = key + 8;
    bool isDown = state != 0;
    const xkb_keysym_t *syms;
    uint32_t numSyms = xkb_key_get_syms(inputDevice->mXkbState, code, &syms);
    xkb_state_update_key(inputDevice->mXkbState, code,
                         isDown ? XKB_KEY_DOWN : XKB_KEY_UP);

    if (!window) {
        // We destroyed the keyboard focus surface, but the server
        // didn't get the message yet.
        return;
    }

    if (numSyms == 1) {
        xkb_keysym_t sym = syms[0];
        Qt::KeyboardModifiers modifiers = translateModifiers(inputDevice->mXkbState);
        QEvent::Type type = isDown ? QEvent::KeyPress : QEvent::KeyRelease;

        char s[2];
        sym = translateKey(sym, s, sizeof s);

        if (window)
            QWindowSystemInterface::handleExtendedKeyEvent(window->window(),
                                                           time, type, sym,
                                                           modifiers,
                                                           code, 0, 0,
                                                           QString::fromLatin1(s));
    }
#else
    // Generic fallback for single hard keys: Assume 'key' is a Qt key code.
    if (window) {
        QWindowSystemInterface::handleExtendedKeyEvent(window->window(),
                                                       time, state ? QEvent::KeyPress : QEvent::KeyRelease,
                                                       key + 8, // qt-compositor substracts 8 for some reason
                                                       Qt::NoModifier,
                                                       key + 8, 0, 0);
    }
#endif
}

void QWaylandInputDevice::keyboard_modifiers(void *data,
                                             struct wl_keyboard *keyboard,
                                             uint32_t serial,
                                             uint32_t mods_depressed,
                                             uint32_t mods_latched,
                                             uint32_t mods_locked,
                                             uint32_t group)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(serial);
#ifndef QT_NO_WAYLAND_XKB
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    if (inputDevice->mXkbState)
        xkb_state_update_mask(inputDevice->mXkbState,
                              mods_depressed, mods_latched, mods_locked,
                              0, 0, group);
#else
    Q_UNUSED(data);
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    Q_UNUSED(group);
#endif
}

void QWaylandInputDevice::touch_down(void *data,
                                     struct wl_touch *touch,
                                     uint32_t serial,
                                     uint32_t time,
                                     struct wl_surface *surface,
                                     int32_t id,
                                     wl_fixed_t x,
                                     wl_fixed_t y)
{
    Q_UNUSED(touch);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->mTouchFocus = static_cast<QWaylandWindow *>(wl_surface_get_user_data(surface));
    inputDevice->handleTouchPoint(id, wl_fixed_to_double(x), wl_fixed_to_double(y), Qt::TouchPointPressed);
}

void QWaylandInputDevice::touch_up(void *data,
                                   struct wl_touch *touch,
                                   uint32_t serial,
                                   uint32_t time,
                                   int32_t id)
{
    Q_UNUSED(touch);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->mTouchFocus = 0;
    inputDevice->handleTouchPoint(id, 0, 0, Qt::TouchPointReleased);
}

void QWaylandInputDevice::touch_motion(void *data,
                                       struct wl_touch *touch,
                                       uint32_t time,
                                       int32_t id,
                                       wl_fixed_t x,
                                       wl_fixed_t y)
{
    Q_UNUSED(touch);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->handleTouchPoint(id, wl_fixed_to_double(x), wl_fixed_to_double(y), Qt::TouchPointMoved);
}

void QWaylandInputDevice::touch_frame(void *data, struct wl_touch *touch)
{
    Q_UNUSED(touch);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->handleTouchFrame();
}

void QWaylandInputDevice::touch_cancel(void *data, struct wl_touch *touch)
{
    Q_UNUSED(touch);
    QWaylandInputDevice *self = static_cast<QWaylandInputDevice *>(data);

    self->mPrevTouchPoints.clear();
    self->mTouchPoints.clear();

    QWaylandTouchExtension *touchExt = self->mQDisplay->touchExtension();
    if (touchExt)
        touchExt->touchCanceled();

    QWindowSystemInterface::handleTouchCancelEvent(0, self->mTouchDevice);
}

void QWaylandInputDevice::handleTouchPoint(int id, double x, double y, Qt::TouchPointState state)
{
    QWindowSystemInterface::TouchPoint tp;

    // Find out the coordinates for Released events.
    bool coordsOk = false;
    if (state == Qt::TouchPointReleased)
        for (int i = 0; i < mPrevTouchPoints.count(); ++i)
            if (mPrevTouchPoints.at(i).id == id) {
                tp.area = mPrevTouchPoints.at(i).area;
                coordsOk = true;
                break;
            }

    if (!coordsOk) {
        // x and y are surface relative.
        // We need a global (screen) position.
        QWaylandWindow *win = mTouchFocus;

        //is it possible that mTouchFocus is null;
        if (!win)
            win = mPointerFocus;
        if (!win)
            win = mKeyboardFocus;
        if (!win || !win->window())
            return;

        tp.area = QRectF(0, 0, 8, 8);
        QMargins margins = win->frameMargins();
        tp.area.moveCenter(win->window()->mapToGlobal(QPoint(x+margins.left(), y+margins.top())));
    }

    tp.state = state;
    tp.id = id;
    tp.pressure = tp.state == Qt::TouchPointReleased ? 0 : 1;
    mTouchPoints.append(tp);
}

void QWaylandInputDevice::handleTouchFrame()
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

    QWindowSystemInterface::handleTouchEvent(0, mTouchDevice, mTouchPoints);

    bool allReleased = true;
    for (int i = 0; i < mTouchPoints.count(); ++i)
        if (mTouchPoints.at(i).state != Qt::TouchPointReleased) {
            allReleased = false;
            break;
        }

    mPrevTouchPoints = mTouchPoints;
    mTouchPoints.clear();

    if (allReleased) {
        QWindowSystemInterface::handleTouchEvent(0, mTouchDevice, mTouchPoints);
        mPrevTouchPoints.clear();
    }
}
