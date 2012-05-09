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

#include <QtGui/QGuiApplication>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#include <X11/keysym.h>
#endif

QWaylandInputDevice::QWaylandInputDevice(QWaylandDisplay *display,
					 uint32_t id)
    : mQDisplay(display)
    , mDisplay(display->wl_display())
    , mTransferDevice(0)
    , mPointerFocus(0)
    , mKeyboardFocus(0)
    , mTouchFocus(0)
    , mButtons(0)
#ifndef QT_NO_WAYLAND_XKB
    , mXkbContext(0)
    , mXkbMap(0)
    , mXkbState(0)
#endif
{
    mInputDevice = static_cast<struct wl_input_device *>
            (wl_display_bind(mDisplay,id,&wl_input_device_interface));
    wl_input_device_add_listener(mInputDevice,
				 &inputDeviceListener,
				 this);
    wl_input_device_set_user_data(mInputDevice, this);

#ifndef QT_NO_WAYLAND_XKB
    xkb_rule_names names;
    names.rules = strdup("evdev");
    names.model = strdup("pc105");
    names.layout = strdup("us");
    names.variant = strdup("");
    names.options = strdup("");

    xkb_context *mXkbContext = xkb_context_new();
    if (mXkbContext) {
        mXkbMap = xkb_map_new_from_names(mXkbContext, &names);
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

    mTouchDevice = new QTouchDevice;
    mTouchDevice->setType(QTouchDevice::TouchScreen);
    mTouchDevice->setCapabilities(QTouchDevice::Position);
    QWindowSystemInterface::registerTouchDevice(mTouchDevice);
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

struct wl_input_device *QWaylandInputDevice::handle() const
{
    return mInputDevice;
}

void QWaylandInputDevice::removeMouseButtonFromState(Qt::MouseButton button)
{
    mButtons = mButtons & !button;
}

void QWaylandInputDevice::inputHandleMotion(void *data,
					    struct wl_input_device *input_device,
					    uint32_t time,
                        wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    Q_UNUSED(input_device);
    Q_UNUSED(surface_x);
    Q_UNUSED(surface_y);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mPointerFocus;

    if (window == NULL) {
	/* We destroyed the pointer focus surface, but the server
	 * didn't get the message yet. */
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
                        inputDevice->mSurfacePos,
                        inputDevice->mButtons,
                        Qt::NoModifier);
}

void QWaylandInputDevice::inputHandleButton(void *data,
					    struct wl_input_device *input_device,
                                            uint32_t serial, uint32_t time,
                                            uint32_t button, uint32_t state)
{
    Q_UNUSED(input_device);
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

    if (window) {
        window->handleMouse(inputDevice,
                            time,
                            inputDevice->mSurfacePos,
                            inputDevice->mSurfacePos,
                            inputDevice->mButtons,
                            Qt::NoModifier);
    }
}

void QWaylandInputDevice::inputHandleAxis(void *data,
                                          struct wl_input_device *wl_input_device,
                                          uint32_t time,
                                          uint32_t axis,
                                          int32_t value)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_input_device);
    Q_UNUSED(time);
    Q_UNUSED(axis);
    Q_UNUSED(value);
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

static uint32_t translateKey(uint32_t sym, char *string, size_t size)
{
    Q_UNUSED(size);
    string[0] = '\0';

    switch (sym) {
    case XK_Escape:		return Qt::Key_Escape;
    case XK_Tab:		return Qt::Key_Tab;
    case XK_ISO_Left_Tab:	return Qt::Key_Backtab;
    case XK_BackSpace:		return Qt::Key_Backspace;
    case XK_Return:		return Qt::Key_Return;
    case XK_Insert:		return Qt::Key_Insert;
    case XK_Delete:		return Qt::Key_Delete;
    case XK_Clear:		return Qt::Key_Delete;
    case XK_Pause:		return Qt::Key_Pause;
    case XK_Print:		return Qt::Key_Print;

    case XK_Home:		return Qt::Key_Home;
    case XK_End:		return Qt::Key_End;
    case XK_Left:		return Qt::Key_Left;
    case XK_Up:			return Qt::Key_Up;
    case XK_Right:		return Qt::Key_Right;
    case XK_Down:		return Qt::Key_Down;
    case XK_Prior:		return Qt::Key_PageUp;
    case XK_Next:		return Qt::Key_PageDown;

    case XK_Shift_L:		return Qt::Key_Shift;
    case XK_Shift_R:		return Qt::Key_Shift;
    case XK_Shift_Lock:		return Qt::Key_Shift;
    case XK_Control_L:		return Qt::Key_Control;
    case XK_Control_R:		return Qt::Key_Control;
    case XK_Meta_L:		return Qt::Key_Meta;
    case XK_Meta_R:		return Qt::Key_Meta;
    case XK_Alt_L:		return Qt::Key_Alt;
    case XK_Alt_R:		return Qt::Key_Alt;
    case XK_Caps_Lock:		return Qt::Key_CapsLock;
    case XK_Num_Lock:		return Qt::Key_NumLock;
    case XK_Scroll_Lock:	return Qt::Key_ScrollLock;
    case XK_Super_L:		return Qt::Key_Super_L;
    case XK_Super_R:		return Qt::Key_Super_R;
    case XK_Menu:		return Qt::Key_Menu;

    default:
	string[0] = sym;
	string[1] = '\0';
	return toupper(sym);
    }
}
#endif

void QWaylandInputDevice::inputHandleKey(void *data,
                                         struct wl_input_device *input_device,
                                         uint32_t serial, uint32_t time,
                                         uint32_t key, uint32_t state)
{
    Q_UNUSED(input_device);
    Q_UNUSED(serial);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window = inputDevice->mKeyboardFocus;
#ifndef QT_NO_WAYLAND_XKB
    uint32_t numSyms, code;
    const xkb_keysym_t *syms;
    Qt::KeyboardModifiers modifiers;
    QEvent::Type type;
    char s[2];

    if (window == NULL || !inputDevice->mXkbMap) {
        // We destroyed the keyboard focus surface, but the server
        // didn't get the message yet.
        return;
    }

    code = key + 8;
    bool isDown = state != 0;
    numSyms = xkb_key_get_syms(inputDevice->mXkbState, code, &syms);
    xkb_state_update_key(inputDevice->mXkbState, code,
                         isDown ? XKB_KEY_DOWN : XKB_KEY_UP);

    xkb_keysym_t sym;
    if (numSyms == 1) {
        sym = syms[0];

        modifiers = translateModifiers(inputDevice->mXkbState);

        if (isDown) {
            inputDevice->mModifiers |= modifiers;
            type = QEvent::KeyPress;
        } else {
            inputDevice->mModifiers &= ~modifiers;
            type = QEvent::KeyRelease;
        }

        sym = translateKey(sym, s, sizeof s);

        if (window)
            QWindowSystemInterface::handleExtendedKeyEvent(window->window(),
                                                           time, type, sym,
                                                           inputDevice->mModifiers,
                                                           code, 0, 0,
                                                           QString::fromLatin1(s));
    }
#else
    // Generic fallback for single hard keys: Assume 'key' is a Qt key code.
    if (window) {
        QWindowSystemInterface::handleExtendedKeyEvent(window->window(),
                                                       time, state ? QEvent::KeyPress : QEvent::KeyRelease,
                                                       key + 8, // qt-compositor substracts 8 for some reason
                                                       inputDevice->mModifiers,
                                                       key + 8, 0, 0);
    }
#endif
}

void QWaylandInputDevice::inputHandlePointerEnter(void *data,
                                                  struct wl_input_device *input_device,
                                                  uint32_t time, struct wl_surface *surface,
                                                  wl_fixed_t sx, wl_fixed_t sy)
{
    Q_UNUSED(input_device);
    Q_UNUSED(sx);
    Q_UNUSED(sy);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;

    // shouldn't get pointer enter with no surface
    Q_ASSERT(surface);

    QWaylandWindow *window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    window->handleMouseEnter();
    inputDevice->mPointerFocus = window;

    inputDevice->mTime = time;
}

void QWaylandInputDevice::inputHandlePointerLeave(void *data,
                                                  struct wl_input_device *input_device,
                                                  uint32_t time, struct wl_surface *surface)
{
    Q_UNUSED(input_device);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;

    // The event may arrive after destroying the window, indicated by
    // a null surface.
    if (!surface)
        return;

    QWaylandWindow *window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    window->handleMouseLeave();
    inputDevice->mPointerFocus = 0;
    inputDevice->mButtons = Qt::NoButton;

    inputDevice->mTime = time;
}

void QWaylandInputDevice::inputHandleKeyboardEnter(void *data,
                                                   struct wl_input_device *input_device,
                                                   uint32_t time,
                                                   struct wl_surface *surface,
                                                   struct wl_array *keys)
{
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    QWaylandWindow *window;

    inputDevice->mModifiers = 0;

    Q_UNUSED(keys);
#ifndef QT_NO_WAYLAND_XKB
    inputDevice->mModifiers |= translateModifiers(inputDevice->mXkbState);
#endif

    // shouldn't get keyboard enter with no surface
    Q_ASSERT(surface);

    window = (QWaylandWindow *) wl_surface_get_user_data(surface);
    inputDevice->mKeyboardFocus = window;
    inputDevice->mQDisplay->setLastKeyboardFocusInputDevice(inputDevice);
    QWindowSystemInterface::handleWindowActivated(window->window());
}

void QWaylandInputDevice::inputHandleKeyboardLeave(void *data,
                                                   struct wl_input_device *input_device,
                                                   uint32_t time,
                                                   struct wl_surface *surface)
{
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    Q_UNUSED(surface);

    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;

    inputDevice->mKeyboardFocus = NULL;
    inputDevice->mQDisplay->setLastKeyboardFocusInputDevice(0);
    QWindowSystemInterface::handleWindowActivated(0);
}

void QWaylandInputDevice::inputHandleTouchDown(void *data,
                                               struct wl_input_device *wl_input_device,
                                               uint32_t serial,
                                               uint32_t time,
                                               struct wl_surface *surface,
                                               int32_t id,
                                               wl_fixed_t x,
                                               wl_fixed_t y)
{
    Q_UNUSED(wl_input_device);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->mTouchFocus = static_cast<QWaylandWindow *>(wl_surface_get_user_data(surface));
    inputDevice->handleTouchPoint(id, wl_fixed_to_double(x), wl_fixed_to_double(y), Qt::TouchPointPressed);
}

void QWaylandInputDevice::inputHandleTouchUp(void *data,
                                             struct wl_input_device *wl_input_device,
                                             uint32_t serial,
                                             uint32_t time,
                                             int32_t id)
{
    Q_UNUSED(wl_input_device);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->mTouchFocus = 0;
    inputDevice->handleTouchPoint(id, 0, 0, Qt::TouchPointReleased);
}

void QWaylandInputDevice::inputHandleTouchMotion(void *data,
                                                 struct wl_input_device *wl_input_device,
                                                 uint32_t time,
                                                 int32_t id,
                                                 wl_fixed_t x,
                                                 wl_fixed_t y)
{
    Q_UNUSED(wl_input_device);
    Q_UNUSED(time);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->handleTouchPoint(id, wl_fixed_to_double(x), wl_fixed_to_double(y), Qt::TouchPointMoved);
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

void QWaylandInputDevice::inputHandleTouchFrame(void *data, struct wl_input_device *wl_input_device)
{
    Q_UNUSED(wl_input_device);
    QWaylandInputDevice *inputDevice = (QWaylandInputDevice *) data;
    inputDevice->handleTouchFrame();
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

void QWaylandInputDevice::inputHandleTouchCancel(void *data, struct wl_input_device *wl_input_device)
{
    Q_UNUSED(wl_input_device);
    QWaylandInputDevice *self = static_cast<QWaylandInputDevice *>(data);

    self->mPrevTouchPoints.clear();
    self->mTouchPoints.clear();

    QWaylandTouchExtension *touchExt = self->mQDisplay->touchExtension();
    if (touchExt)
        touchExt->touchCanceled();

    QWindowSystemInterface::handleTouchCancelEvent(0, self->mTouchDevice);
}

const struct wl_input_device_listener QWaylandInputDevice::inputDeviceListener = {
    QWaylandInputDevice::inputHandleMotion,
    QWaylandInputDevice::inputHandleButton,
    QWaylandInputDevice::inputHandleAxis,
    QWaylandInputDevice::inputHandleKey,
    QWaylandInputDevice::inputHandlePointerEnter,
    QWaylandInputDevice::inputHandlePointerLeave,
    QWaylandInputDevice::inputHandleKeyboardEnter,
    QWaylandInputDevice::inputHandleKeyboardLeave,
    QWaylandInputDevice::inputHandleTouchDown,
    QWaylandInputDevice::inputHandleTouchUp,
    QWaylandInputDevice::inputHandleTouchMotion,
    QWaylandInputDevice::inputHandleTouchFrame,
    QWaylandInputDevice::inputHandleTouchCancel
};

void QWaylandInputDevice::attach(QWaylandBuffer *buffer, int x, int y)
{
    wl_input_device_attach(mInputDevice, mTime, buffer->buffer(), x, y);
}
