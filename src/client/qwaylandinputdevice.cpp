/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwaylandinputdevice_p.h"

#include "qwaylandintegration_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylandbuffer_p.h"
#if QT_CONFIG(wayland_datadevice)
#include "qwaylanddatadevice_p.h"
#include "qwaylanddatadevicemanager_p.h"
#endif
#include "qwaylandtouch_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandcursor_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandshmbackingstore_p.h"
#include "qwaylandinputcontext_p.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatforminputcontext.h>
#include <QDebug>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#if QT_CONFIG(cursor)
#include <wayland-cursor.h>
#endif

#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandInputDevice::Keyboard::Keyboard(QWaylandInputDevice *p)
    : mParent(p)
{
    mRepeatTimer.callOnTimeout([&]() {
        if (!focusWindow()) {
            // We destroyed the keyboard focus surface, but the server didn't get the message yet...
            // or the server didn't send an enter event first.
            return;
        }
        mRepeatTimer.setInterval(mRepeatRate);
        handleKey(mRepeatKey.time, QEvent::KeyRelease, mRepeatKey.key, mRepeatKey.modifiers,
                  mRepeatKey.code, mRepeatKey.nativeVirtualKey, mRepeatKey.nativeModifiers,
                  mRepeatKey.text, true);
        handleKey(mRepeatKey.time, QEvent::KeyPress, mRepeatKey.key, mRepeatKey.modifiers,
                  mRepeatKey.code, mRepeatKey.nativeVirtualKey, mRepeatKey.nativeModifiers,
                  mRepeatKey.text, true);
    });
}

#if QT_CONFIG(xkbcommon)
bool QWaylandInputDevice::Keyboard::createDefaultKeymap()
{
    struct xkb_context *ctx = mParent->mQDisplay->xkbContext();
    if (!ctx)
        return false;

    struct xkb_rule_names names;
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";

    mXkbKeymap.reset(xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (mXkbKeymap)
        mXkbState.reset(xkb_state_new(mXkbKeymap.get()));

    if (!mXkbKeymap || !mXkbState) {
        qCWarning(lcQpaWayland, "failed to create default keymap");
        return false;
    }

    return true;
}
#endif

QWaylandInputDevice::Keyboard::~Keyboard()
{
    if (mFocus)
        QWindowSystemInterface::handleWindowActivated(nullptr);
    if (mParent->mVersion >= 3)
        wl_keyboard_release(object());
    else
        wl_keyboard_destroy(object());
}

QWaylandWindow *QWaylandInputDevice::Keyboard::focusWindow() const
{
    return mFocus ? QWaylandWindow::fromWlSurface(mFocus) : nullptr;
}

QWaylandInputDevice::Pointer::Pointer(QWaylandInputDevice *seat)
    : mParent(seat)
{
}

QWaylandInputDevice::Pointer::~Pointer()
{
    if (mParent->mVersion >= 3)
        wl_pointer_release(object());
    else
        wl_pointer_destroy(object());
}

#if QT_CONFIG(cursor)

class CursorSurface : public QObject, public QtWayland::wl_surface
{
public:
    explicit CursorSurface(QWaylandInputDevice::Pointer *pointer, QWaylandDisplay *display)
        : m_pointer(pointer)
    {
        init(display->createSurface(this));
        //TODO: When we upgrade to libwayland 1.10, use wl_surface_get_version instead.
        m_version = display->compositorVersion();
        connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen *screen) {
            int oldScale = outputScale();
            if (!m_screens.removeOne(static_cast<QWaylandScreen *>(screen->handle())))
                return;

            if (outputScale() != oldScale)
                m_pointer->updateCursor();
        });
    }

    void hide()
    {
        uint serial = m_pointer->mEnterSerial;
        Q_ASSERT(serial);
        m_pointer->set_cursor(serial, nullptr, 0, 0);
        m_setSerial = 0;
    }

    // Size and hotspot are in surface coordinates
    void update(wl_buffer *buffer, const QPoint &hotspot, const QSize &size, int bufferScale)
    {
        // Calling code needs to ensure buffer scale is supported if != 1
        Q_ASSERT(bufferScale == 1 || m_version >= 3);

        auto enterSerial = m_pointer->mEnterSerial;
        if (m_setSerial < enterSerial || m_hotspot != hotspot) {
            m_pointer->set_cursor(m_pointer->mEnterSerial, object(), hotspot.x(), hotspot.y());
            m_setSerial = enterSerial;
            m_hotspot = hotspot;
        }

        if (m_version >= 3)
            set_buffer_scale(bufferScale);

        attach(buffer, 0, 0);
        damage(0, 0, size.width(), size.height());
        commit();
    }

    int outputScale() const
    {
        int scale = 0;
        for (auto *screen : m_screens)
            scale = qMax(scale, screen->scale());
        return scale;
    }

protected:
    void surface_enter(struct ::wl_output *output) override
    {
        int oldScale = outputScale();
        auto *screen = QWaylandScreen::fromWlOutput(output);
        if (m_screens.contains(screen))
            return;

        m_screens.append(screen);

        if (outputScale() != oldScale)
            m_pointer->updateCursor();
    }

    void surface_leave(struct ::wl_output *output) override
    {
        int oldScale = outputScale();
        auto *screen = QWaylandScreen::fromWlOutput(output);

        if (!m_screens.removeOne(screen))
            return;

        if (outputScale() != oldScale)
            m_pointer->updateCursor();
    }

private:
    QWaylandInputDevice::Pointer *m_pointer = nullptr;
    uint m_version = 0;
    uint m_setSerial = 0;
    QPoint m_hotspot;
    QVector<QWaylandScreen *> m_screens;
};

QString QWaylandInputDevice::Pointer::cursorThemeName() const
{
    static QString themeName = qEnvironmentVariable("XCURSOR_THEME", QStringLiteral("default"));
    return themeName;
}

int QWaylandInputDevice::Pointer::cursorSize() const
{
    constexpr int defaultCursorSize = 32;
    static const int xCursorSize = qEnvironmentVariableIntValue("XCURSOR_SIZE");
    return xCursorSize > 0 ? xCursorSize : defaultCursorSize;
}

int QWaylandInputDevice::Pointer::idealCursorScale() const
{
    // set_buffer_scale is not supported on earlier versions
    if (seat()->mQDisplay->compositorVersion() < 3)
        return 1;

    if (auto *s = mCursor.surface.data()) {
        if (s->outputScale() > 0)
            return s->outputScale();
    }

    return seat()->mCursor.fallbackOutputScale;
}

void QWaylandInputDevice::Pointer::updateCursorTheme()
{
    int scale = idealCursorScale();
    int pixelSize = cursorSize() * scale;
    auto *display = seat()->mQDisplay;
    mCursor.theme = display->loadCursorTheme(cursorThemeName(), pixelSize);

    if (!mCursor.theme)
        return; // A warning has already been printed in loadCursorTheme

    if (auto *arrow = mCursor.theme->cursorImage(Qt::ArrowCursor)) {
        int arrowPixelSize = qMax(arrow->width, arrow->height); // Not all cursor themes are square
        while (scale > 1 && arrowPixelSize / scale < cursorSize())
            --scale;
    } else {
        qCWarning(lcQpaWayland) << "Cursor theme does not support the arrow cursor";
    }
    mCursor.themeBufferScale = scale;
}

void QWaylandInputDevice::Pointer::updateCursor()
{
    if (mEnterSerial == 0)
        return;

    auto shape = seat()->mCursor.shape;

    if (shape == Qt::BlankCursor) {
        if (mCursor.surface)
            mCursor.surface->hide();
        return;
    }

    if (shape == Qt::BitmapCursor) {
        auto buffer = seat()->mCursor.bitmapBuffer;
        if (!buffer) {
            qCWarning(lcQpaWayland) << "No buffer for bitmap cursor, can't set cursor";
            return;
        }
        auto hotspot = seat()->mCursor.hotspot;
        int bufferScale = seat()->mCursor.bitmapScale;
        getOrCreateCursorSurface()->update(buffer->buffer(), hotspot, buffer->size(), bufferScale);
        return;
    }

    if (!mCursor.theme || idealCursorScale() != mCursor.themeBufferScale)
        updateCursorTheme();

    if (!mCursor.theme)
        return;

    // Set from shape using theme
    if (struct ::wl_cursor_image *image = mCursor.theme->cursorImage(shape)) {
        struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
        int bufferScale = mCursor.themeBufferScale;
        QPoint hotspot = QPoint(image->hotspot_x, image->hotspot_y) / bufferScale;
        QSize size = QSize(image->width, image->height) / bufferScale;
        getOrCreateCursorSurface()->update(buffer, hotspot, size, bufferScale);
        return;
    }

    qCWarning(lcQpaWayland) << "Unable to change to cursor" << shape;
}

CursorSurface *QWaylandInputDevice::Pointer::getOrCreateCursorSurface()
{
    if (!mCursor.surface)
        mCursor.surface.reset(new CursorSurface(this, seat()->mQDisplay));
    return mCursor.surface.get();
}

#endif // QT_CONFIG(cursor)

QWaylandInputDevice::Touch::Touch(QWaylandInputDevice *p)
    : mParent(p)
{
}

QWaylandInputDevice::Touch::~Touch()
{
    if (mParent->mVersion >= 3)
        wl_touch_release(object());
    else
        wl_touch_destroy(object());
}

QWaylandInputDevice::QWaylandInputDevice(QWaylandDisplay *display, int version, uint32_t id)
    : QtWayland::wl_seat(display->wl_registry(), id, qMin(version, 4))
    , mQDisplay(display)
    , mDisplay(display->wl_display())
    , mVersion(qMin(version, 4))
{
#if QT_CONFIG(wayland_datadevice)
    if (mQDisplay->dndSelectionHandler()) {
        mDataDevice = mQDisplay->dndSelectionHandler()->getDataDevice(this);
    }
#endif

    if (mQDisplay->textInputManager())
        mTextInput.reset(new QWaylandTextInput(mQDisplay, mQDisplay->textInputManager()->get_text_input(wl_seat())));

}

QWaylandInputDevice::~QWaylandInputDevice()
{
    delete mPointer;
    delete mKeyboard;
    delete mTouch;
}

void QWaylandInputDevice::seat_capabilities(uint32_t caps)
{
    mCaps = caps;

    if (caps & WL_SEAT_CAPABILITY_KEYBOARD && !mKeyboard) {
        mKeyboard = createKeyboard(this);
        mKeyboard->init(get_keyboard());
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && mKeyboard) {
        delete mKeyboard;
        mKeyboard = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_POINTER && !mPointer) {
        mPointer = createPointer(this);
        mPointer->init(get_pointer());
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && mPointer) {
        delete mPointer;
        mPointer = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_TOUCH && !mTouch) {
        mTouch = createTouch(this);
        mTouch->init(get_touch());

        if (!mTouchDevice) {
            mTouchDevice = new QTouchDevice;
            mTouchDevice->setType(QTouchDevice::TouchScreen);
            mTouchDevice->setCapabilities(QTouchDevice::Position);
            QWindowSystemInterface::registerTouchDevice(mTouchDevice);
        }
    } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && mTouch) {
        delete mTouch;
        mTouch = nullptr;
    }
}

QWaylandInputDevice::Keyboard *QWaylandInputDevice::createKeyboard(QWaylandInputDevice *device)
{
    return new Keyboard(device);
}

QWaylandInputDevice::Pointer *QWaylandInputDevice::createPointer(QWaylandInputDevice *device)
{
    return new Pointer(device);
}

QWaylandInputDevice::Touch *QWaylandInputDevice::createTouch(QWaylandInputDevice *device)
{
    return new Touch(device);
}

void QWaylandInputDevice::handleEndDrag()
{
    if (mTouch)
        mTouch->releasePoints();
    if (mPointer)
        mPointer->releaseButtons();
}

#if QT_CONFIG(wayland_datadevice)
void QWaylandInputDevice::setDataDevice(QWaylandDataDevice *device)
{
    mDataDevice = device;
}

QWaylandDataDevice *QWaylandInputDevice::dataDevice() const
{
    return mDataDevice;
}
#endif

void QWaylandInputDevice::setTextInput(QWaylandTextInput *textInput)
{
    mTextInput.reset(textInput);
}

QWaylandTextInput *QWaylandInputDevice::textInput() const
{
    return mTextInput.data();
}

void QWaylandInputDevice::removeMouseButtonFromState(Qt::MouseButton button)
{
    if (mPointer)
        mPointer->mButtons = mPointer->mButtons & !button;
}

QWaylandWindow *QWaylandInputDevice::pointerFocus() const
{
    return mPointer ? mPointer->mFocus : nullptr;
}

QWaylandWindow *QWaylandInputDevice::keyboardFocus() const
{
    return mKeyboard ? mKeyboard->focusWindow() : nullptr;
}

QWaylandWindow *QWaylandInputDevice::touchFocus() const
{
    return mTouch ? mTouch->mFocus : nullptr;
}

QPointF QWaylandInputDevice::pointerSurfacePosition() const
{
    return mPointer ? mPointer->mSurfacePos : QPointF();
}

QList<int> QWaylandInputDevice::possibleKeys(const QKeyEvent *event) const
{
#if QT_CONFIG(xkbcommon)
    if (mKeyboard && mKeyboard->mXkbState)
        return QXkbCommon::possibleKeys(mKeyboard->mXkbState.get(), event);
#else
    Q_UNUSED(event);
#endif
    return {};
}

Qt::KeyboardModifiers QWaylandInputDevice::modifiers() const
{
    if (!mKeyboard)
        return Qt::NoModifier;

    return mKeyboard->modifiers();
}

Qt::KeyboardModifiers QWaylandInputDevice::Keyboard::modifiers() const
{
    Qt::KeyboardModifiers ret = Qt::NoModifier;

#if QT_CONFIG(xkbcommon)
    if (!mXkbState)
        return ret;

    ret = QXkbCommon::modifiers(mXkbState.get());
#endif

    return ret;
}

#if QT_CONFIG(cursor)
void QWaylandInputDevice::setCursor(const QCursor *cursor, const QSharedPointer<QWaylandBuffer> &cachedBuffer, int fallbackOutputScale)
{
    CursorState oldCursor = mCursor;
    mCursor = CursorState(); // Clear any previous state
    mCursor.shape = cursor ? cursor->shape() : Qt::ArrowCursor;
    mCursor.hotspot = cursor ? cursor->hotSpot() : QPoint();
    mCursor.fallbackOutputScale = fallbackOutputScale;

    if (mCursor.shape == Qt::BitmapCursor) {
        mCursor.bitmapBuffer = cachedBuffer ? cachedBuffer : QWaylandCursor::cursorBitmapBuffer(mQDisplay, cursor);
        qreal dpr = cursor->pixmap().devicePixelRatio();
        mCursor.bitmapScale = int(dpr); // Wayland doesn't support fractional buffer scale
        // If there was a fractional part of the dpr, we need to scale the hotspot accordingly
        if (mCursor.bitmapScale < dpr)
            mCursor.hotspot *= dpr / mCursor.bitmapScale;
    }

    // Return early if setCursor was called redundantly (mostly happens from decorations)
    if (mCursor.shape != Qt::BitmapCursor
            && mCursor.shape == oldCursor.shape
            && mCursor.hotspot == oldCursor.hotspot
            && mCursor.fallbackOutputScale == oldCursor.fallbackOutputScale) {
        return;
    }

    if (mPointer)
        mPointer->updateCursor();
}
#endif

class EnterEvent : public QWaylandPointerEvent
{
public:
    EnterEvent(const QPointF &l, const QPointF &g)
        : QWaylandPointerEvent(QWaylandPointerEvent::Enter, 0, l, g, nullptr, Qt::NoModifier)
    {}
};

void QWaylandInputDevice::Pointer::pointer_enter(uint32_t serial, struct wl_surface *surface,
                                                 wl_fixed_t sx, wl_fixed_t sy)
{
    if (!surface)
        return;

    QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    if (mFocus) {
        qCWarning(lcQpaWayland) << "The compositor sent a wl_pointer.enter event before sending a"
                                << "leave event first, this is not allowed by the wayland protocol"
                                << "attempting to work around it by invalidating the current focus";
        invalidateFocus();
    }

    mFocus = window;
    connect(mFocus.data(), &QWaylandWindow::wlSurfaceDestroyed, this, &Pointer::handleFocusDestroyed);

    mSurfacePos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    mGlobalPos = window->window()->mapToGlobal(mSurfacePos.toPoint());

    mParent->mSerial = serial;
    mEnterSerial = serial;

#if QT_CONFIG(cursor)
    // Depends on mEnterSerial being updated
    updateCursor();
#endif

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (!grab) {
        EnterEvent evt(mSurfacePos, mGlobalPos);
        window->handleMouse(mParent, evt);
    }
}

void QWaylandInputDevice::Pointer::pointer_leave(uint32_t time, struct wl_surface *surface)
{
    // The event may arrive after destroying the window, indicated by
    // a null surface.
    if (!surface)
        return;

    auto *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    if (!QWaylandWindow::mouseGrab()) {
        QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
        window->handleMouseLeave(mParent);
    }

    invalidateFocus();
    mButtons = Qt::NoButton;

    mParent->mTime = time;
}

class MotionEvent : public QWaylandPointerEvent
{
public:
    MotionEvent(ulong t, const QPointF &l, const QPointF &g, Qt::MouseButtons b, Qt::KeyboardModifiers m)
        : QWaylandPointerEvent(QWaylandPointerEvent::Motion, t, l, g, b, m)
    {
    }
};

void QWaylandInputDevice::Pointer::pointer_motion(uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    QPointF pos(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
    QPointF delta = pos - pos.toPoint();
    QPointF global = window->window()->mapToGlobal(pos.toPoint());
    global += delta;

    mSurfacePos = pos;
    mGlobalPos = global;
    mParent->mTime = time;

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (grab && grab != window) {
        // We can't know the true position since we're getting events for another surface,
        // so we just set it outside of the window boundaries.
        pos = QPointF(-1, -1);
        global = grab->window()->mapToGlobal(pos.toPoint());
        MotionEvent e(time, pos, global, mButtons, mParent->modifiers());
        grab->handleMouse(mParent, e);
    } else {
        MotionEvent e(time, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
        window->handleMouse(mParent, e);
    }
}

void QWaylandInputDevice::Pointer::pointer_button(uint32_t serial, uint32_t time,
                                                  uint32_t button, uint32_t state)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

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
        mButtons |= qt_button;
    else
        mButtons &= ~qt_button;

    mParent->mTime = time;
    mParent->mSerial = serial;
    if (state)
        mParent->mQDisplay->setLastInputDevice(mParent, serial, window);

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (grab && grab != mFocus) {
        QPointF pos = QPointF(-1, -1);
        QPointF global = grab->window()->mapToGlobal(pos.toPoint());
        MotionEvent e(time, pos, global, mButtons, mParent->modifiers());
        grab->handleMouse(mParent, e);
    } else if (window) {
        MotionEvent e(time, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
        window->handleMouse(mParent, e);
    }
}

void QWaylandInputDevice::Pointer::invalidateFocus()
{
    disconnect(mFocus.data(), &QWaylandWindow::wlSurfaceDestroyed, this, &Pointer::handleFocusDestroyed);
    mFocus = nullptr;
    mEnterSerial = 0;
}

void QWaylandInputDevice::Pointer::releaseButtons()
{
    mButtons = Qt::NoButton;
    MotionEvent e(mParent->mTime, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
    if (mFocus)
        mFocus->handleMouse(mParent, e);
}

class WheelEvent : public QWaylandPointerEvent
{
public:
    WheelEvent(ulong t, const QPointF &l, const QPointF &g, const QPoint &pd, const QPoint &ad, Qt::KeyboardModifiers m)
        : QWaylandPointerEvent(QWaylandPointerEvent::Wheel, t, l, g, pd, ad, m)
    {
    }
};

void QWaylandInputDevice::Pointer::pointer_axis(uint32_t time, uint32_t axis, int32_t value)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

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

    WheelEvent e(time, mSurfacePos, mGlobalPos, pixelDelta, angleDelta, mParent->modifiers());
    window->handleMouse(mParent, e);
}

void QWaylandInputDevice::Keyboard::keyboard_keymap(uint32_t format, int32_t fd, uint32_t size)
{
    mKeymapFormat = format;
#if QT_CONFIG(xkbcommon)
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        qCWarning(lcQpaWayland) << "unknown keymap format:" << format;
        close(fd);
        return;
    }

    char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }

    mXkbKeymap.reset(xkb_keymap_new_from_string(mParent->mQDisplay->xkbContext(), map_str,
                                                XKB_KEYMAP_FORMAT_TEXT_V1,
                                                XKB_KEYMAP_COMPILE_NO_FLAGS));
    QXkbCommon::verifyHasLatinLayout(mXkbKeymap.get());

    munmap(map_str, size);
    close(fd);

    if (mXkbKeymap)
        mXkbState.reset(xkb_state_new(mXkbKeymap.get()));
    else
        mXkbState.reset(nullptr);
#else
    Q_UNUSED(fd);
    Q_UNUSED(size);
#endif
}

void QWaylandInputDevice::Keyboard::keyboard_enter(uint32_t time, struct wl_surface *surface, struct wl_array *keys)
{
    Q_UNUSED(time);
    Q_UNUSED(keys);

    if (!surface) {
        // Ignoring wl_keyboard.enter event with null surface. This is either a compositor bug,
        // or it's a race with a wl_surface.destroy request. In either case, ignore the event.
        return;
    }

    if (mFocus) {
        qCWarning(lcQpaWayland()) << "Unexpected wl_keyboard.enter event. Keyboard already has focus";
        disconnect(focusWindow(), &QWaylandWindow::wlSurfaceDestroyed, this, &Keyboard::handleFocusDestroyed);
    }

    mFocus = surface;
    connect(focusWindow(), &QWaylandWindow::wlSurfaceDestroyed, this, &Keyboard::handleFocusDestroyed);

    mParent->mQDisplay->handleKeyboardFocusChanged(mParent);
}

void QWaylandInputDevice::Keyboard::keyboard_leave(uint32_t time, struct wl_surface *surface)
{
    Q_UNUSED(time);

    if (!surface) {
        // Either a compositor bug, or a race condition with wl_surface.destroy, ignore the event.
        return;
    }

    if (surface != mFocus) {
        qCWarning(lcQpaWayland) << "Ignoring unexpected wl_keyboard.leave event."
                                << "wl_surface argument does not match the current focus"
                                << "This is most likely a compositor bug";
        return;
    }
    disconnect(focusWindow(), &QWaylandWindow::wlSurfaceDestroyed, this, &Keyboard::handleFocusDestroyed);
    handleFocusLost();
}

void QWaylandInputDevice::Keyboard::handleKey(ulong timestamp, QEvent::Type type, int key,
                                              Qt::KeyboardModifiers modifiers, quint32 nativeScanCode,
                                              quint32 nativeVirtualKey, quint32 nativeModifiers,
                                              const QString &text, bool autorepeat, ushort count)
{
    QPlatformInputContext *inputContext = QGuiApplicationPrivate::platformIntegration()->inputContext();
    bool filtered = false;

    if (inputContext && !mParent->mQDisplay->usingInputContextFromCompositor()) {
        QKeyEvent event(type, key, modifiers, nativeScanCode, nativeVirtualKey,
                        nativeModifiers, text, autorepeat, count);
        event.setTimestamp(timestamp);
        filtered = inputContext->filterEvent(&event);
    }

    if (!filtered) {
        QWindowSystemInterface::handleExtendedKeyEvent(focusWindow()->window(), timestamp, type, key, modifiers,
                nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorepeat, count);
    }
}

void QWaylandInputDevice::Keyboard::keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (mKeymapFormat != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 && mKeymapFormat != WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP) {
        qCWarning(lcQpaWayland) << Q_FUNC_INFO << "unknown keymap format:" << mKeymapFormat;
        return;
    }

    auto *window = focusWindow();
    if (!window) {
        // We destroyed the keyboard focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    mParent->mSerial = serial;

    const bool isDown = state != WL_KEYBOARD_KEY_STATE_RELEASED;
    if (isDown)
        mParent->mQDisplay->setLastInputDevice(mParent, serial, window);

    if (mKeymapFormat == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
#if QT_CONFIG(xkbcommon)
        if ((!mXkbKeymap || !mXkbState) && !createDefaultKeymap())
            return;

        auto code = key + 8; // map to wl_keyboard::keymap_format::keymap_format_xkb_v1

        xkb_keysym_t sym = xkb_state_key_get_one_sym(mXkbState.get(), code);

        Qt::KeyboardModifiers modifiers = mParent->modifiers();

        int qtkey = QXkbCommon::keysymToQtKey(sym, modifiers, mXkbState.get(), code);
        QString text = QXkbCommon::lookupString(mXkbState.get(), code);

        QEvent::Type type = isDown ? QEvent::KeyPress : QEvent::KeyRelease;
        handleKey(time, type, qtkey, modifiers, code, sym, mNativeModifiers, text);

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED && xkb_keymap_key_repeats(mXkbKeymap.get(), code) && mRepeatRate > 0) {
            mRepeatKey.key = qtkey;
            mRepeatKey.code = code;
            mRepeatKey.time = time;
            mRepeatKey.text = text;
            mRepeatKey.modifiers = modifiers;
            mRepeatKey.nativeModifiers = mNativeModifiers;
            mRepeatKey.nativeVirtualKey = sym;
            mRepeatTimer.setInterval(mRepeatDelay);
            mRepeatTimer.start();
        } else if (mRepeatKey.code == code) {
            mRepeatTimer.stop();
        }
#else
        Q_UNUSED(time);
        Q_UNUSED(key);
        qCWarning(lcQpaWayland, "xkbcommon not available on this build, not performing key mapping");
        return;
#endif
    } else if (mKeymapFormat == WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP) {
        // raw scan code
        return;
    }
}

void QWaylandInputDevice::Keyboard::handleFocusDestroyed()
{
    // The signal is emitted by QWaylandWindow, which is not necessarily destroyed along with the
    // surface, so we still need to disconnect the signal
    auto *window = qobject_cast<QWaylandWindow *>(sender());
    disconnect(window, &QWaylandWindow::wlSurfaceDestroyed, this, &Keyboard::handleFocusDestroyed);
    Q_ASSERT(window->object() == mFocus);
    handleFocusLost();
}

void QWaylandInputDevice::Keyboard::handleFocusLost()
{
    mFocus = nullptr;
#if QT_CONFIG(clipboard)
    if (auto *dataDevice = mParent->dataDevice())
        dataDevice->invalidateSelectionOffer();
#endif
    mParent->mQDisplay->handleKeyboardFocusChanged(mParent);
    mRepeatTimer.stop();
}

void QWaylandInputDevice::Keyboard::keyboard_modifiers(uint32_t serial,
                                             uint32_t mods_depressed,
                                             uint32_t mods_latched,
                                             uint32_t mods_locked,
                                             uint32_t group)
{
    Q_UNUSED(serial);
#if QT_CONFIG(xkbcommon)
    if (mXkbState)
        xkb_state_update_mask(mXkbState.get(),
                              mods_depressed, mods_latched, mods_locked,
                              0, 0, group);
    mNativeModifiers = mods_depressed | mods_latched | mods_locked;
#else
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    Q_UNUSED(group);
#endif
}

void QWaylandInputDevice::Keyboard::keyboard_repeat_info(int32_t rate, int32_t delay)
{
    mRepeatRate = rate;
    mRepeatDelay = delay;
}

void QWaylandInputDevice::Touch::touch_down(uint32_t serial,
                                     uint32_t time,
                                     struct wl_surface *surface,
                                     int32_t id,
                                     wl_fixed_t x,
                                     wl_fixed_t y)
{
    if (!surface)
        return;

    auto *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    mParent->mTime = time;
    mParent->mSerial = serial;
    mFocus = window;
    mParent->mQDisplay->setLastInputDevice(mParent, serial, mFocus);
    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    mParent->handleTouchPoint(id, Qt::TouchPointPressed, position);
}

void QWaylandInputDevice::Touch::touch_up(uint32_t serial, uint32_t time, int32_t id)
{
    Q_UNUSED(serial);
    Q_UNUSED(time);
    mParent->handleTouchPoint(id, Qt::TouchPointReleased);

    if (allTouchPointsReleased()) {
        mFocus = nullptr;

        // As of Weston 7.0.0 there is no touch_frame after the last touch_up
        // (i.e. when the last finger is released). To accommodate for this, issue a
        // touch_frame. This cannot hurt since it is safe to call the touch_frame
        // handler multiple times when there are no points left.
        // See: https://gitlab.freedesktop.org/wayland/weston/issues/44
        // TODO: change logging category to lcQpaWaylandInput in newer versions.
        qCDebug(lcQpaWayland, "Generating fake frame event to work around Weston bug");
        touch_frame();
    }
}

void QWaylandInputDevice::Touch::touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(time);
    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    mParent->handleTouchPoint(id, Qt::TouchPointMoved, position);
}

void QWaylandInputDevice::Touch::touch_cancel()
{
    mPendingTouchPoints.clear();

    QWaylandTouchExtension *touchExt = mParent->mQDisplay->touchExtension();
    if (touchExt)
        touchExt->touchCanceled();

    QWindowSystemInterface::handleTouchCancelEvent(nullptr, mParent->mTouchDevice);
}

void QWaylandInputDevice::handleTouchPoint(int id, Qt::TouchPointState state, const QPointF &surfacePosition)
{
    auto end = mTouch->mPendingTouchPoints.end();
    auto it = std::find_if(mTouch->mPendingTouchPoints.begin(), end, [id](auto tp){ return tp.id == id; });
    if (it == end) {
        it = mTouch->mPendingTouchPoints.insert(end, QWindowSystemInterface::TouchPoint());
        it->id = id;
    }
    QWindowSystemInterface::TouchPoint &tp = *it;

    // Only moved and pressed needs to update/set position
    if (state == Qt::TouchPointMoved || state == Qt::TouchPointPressed) {
        // We need a global (screen) position.
        QWaylandWindow *win = mTouch->mFocus;

        //is it possible that mTouchFocus is null;
        if (!win && mPointer)
            win = mPointer->mFocus;
        if (!win && mKeyboard)
            win = mKeyboard->focusWindow();
        if (!win || !win->window())
            return;

        tp.area = QRectF(0, 0, 8, 8);
        QMargins margins = win->frameMargins();
        QPointF localPosition = surfacePosition - QPointF(margins.left(), margins.top());
        // TODO: This doesn't account for high dpi scaling for the delta, but at least it matches
        // what we have for mouse input.
        QPointF delta = localPosition - localPosition.toPoint();
        QPointF globalPosition = win->window()->mapToGlobal(localPosition.toPoint()) + delta;
        tp.area.moveCenter(globalPosition);
    }

    tp.state = state;
    tp.pressure = tp.state == Qt::TouchPointReleased ? 0 : 1;
}

bool QWaylandInputDevice::Touch::allTouchPointsReleased()
{
    for (const auto &tp : qAsConst(mPendingTouchPoints)) {
        if (tp.state != Qt::TouchPointReleased)
            return false;
    }
    return true;
}

void QWaylandInputDevice::Touch::releasePoints()
{
    if (mPendingTouchPoints.empty())
        return;

    for (QWindowSystemInterface::TouchPoint &tp : mPendingTouchPoints)
        tp.state = Qt::TouchPointReleased;

    touch_frame();
}

void QWaylandInputDevice::Touch::touch_frame()
{
    // TODO: early return if no events?

    QWindow *window = mFocus ? mFocus->window() : nullptr;

    if (mFocus) {
        const QWindowSystemInterface::TouchPoint &tp = mPendingTouchPoints.last();
        // When the touch event is received, the global pos is calculated with the margins
        // in mind. Now we need to adjust again to get the correct local pos back.
        QMargins margins = window->frameMargins();
        QPoint p = tp.area.center().toPoint();
        QPointF localPos(window->mapFromGlobal(QPoint(p.x() + margins.left(), p.y() + margins.top())));
        if (mFocus->touchDragDecoration(mParent, localPos, tp.area.center(), tp.state, mParent->modifiers()))
            return;
    }

    QWindowSystemInterface::handleTouchEvent(window, mParent->mTouchDevice, mPendingTouchPoints);

    // Prepare state for next frame
    const auto prevTouchPoints = mPendingTouchPoints;
    mPendingTouchPoints.clear();
    for (const auto &prevPoint: prevTouchPoints) {
        // All non-released touch points should be part of the next touch event
        if (prevPoint.state != Qt::TouchPointReleased) {
            QWindowSystemInterface::TouchPoint tp = prevPoint;
            tp.state = Qt::TouchPointStationary; // ... as stationary (unless proven otherwise)
            mPendingTouchPoints.append(tp);
        }
    }

}

}

QT_END_NAMESPACE
