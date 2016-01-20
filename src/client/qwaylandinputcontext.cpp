/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
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


#include "qwaylandinputcontext_p.h"

#include <QGuiApplication>
#include <QWindow>
#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif

#include "qwaylanddisplay_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandwindow_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

static Qt::Key toQtKey(uint32_t sym)
{
#ifndef QT_NO_WAYLAND_XKB
    switch (static_cast<xkb_keysym_t>(sym)) {
    case XKB_KEY_BackSpace:
        return Qt::Key_Backspace;
    case XKB_KEY_Return:
        return Qt::Key_Return;
    case XKB_KEY_Left:
        return Qt::Key_Left;
    case XKB_KEY_Up:
        return Qt::Key_Up;
    case XKB_KEY_Right:
        return Qt::Key_Right;
    case XKB_KEY_Down:
        return Qt::Key_Down;
    default:
        return Qt::Key_unknown;
    }
#else
    Q_UNUSED(sym)
    return Qt::Key_unknown;
#endif
}

static QEvent::Type toQEventType(uint32_t state)
{
    switch (static_cast<wl_keyboard_key_state>(state)) {
    default:
    case WL_KEYBOARD_KEY_STATE_PRESSED:
        return QEvent::KeyPress;
    case WL_KEYBOARD_KEY_STATE_RELEASED:
        return QEvent::KeyRelease;
    }
}

QWaylandTextInput::QWaylandTextInput(struct ::wl_text_input *text_input)
    : QtWayland::wl_text_input(text_input)
    , m_commit()
    , m_serial(0)
    , m_resetSerial(0)
{
}

QString QWaylandTextInput::commitString() const
{
    return m_commit;
}

void QWaylandTextInput::reset()
{
    wl_text_input::reset();
    updateState();
    m_resetSerial = m_serial;
}

void QWaylandTextInput::updateState()
{
    if (!QGuiApplication::focusObject())
        return;

    QInputMethodQueryEvent event(Qt::ImQueryAll);
    QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

    const QString &text = event.value(Qt::ImSurroundingText).toString();
    const int cursor = event.value(Qt::ImCursorPosition).toInt();
    const int anchor = event.value(Qt::ImAnchorPosition).toInt();

    set_surrounding_text(text, text.leftRef(cursor).toUtf8().size(), text.leftRef(anchor).toUtf8().size());

    commit_state(++m_serial);
}

void QWaylandTextInput::text_input_preedit_string(uint32_t serial, const QString &text, const QString &commit)
{
    Q_UNUSED(serial)
    if (!QGuiApplication::focusObject())
        return;

    m_commit = commit;
    QList<QInputMethodEvent::Attribute> attributes;
    QInputMethodEvent event(text, attributes);
    QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);
}

void QWaylandTextInput::text_input_commit_string(uint32_t serial, const QString &text)
{
    Q_UNUSED(serial);
    if (!QGuiApplication::focusObject())
        return;

    QInputMethodEvent event;
    event.setCommitString(text);
    QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

    m_commit = QString();
}

void QWaylandTextInput::text_input_enter(wl_surface *)
{
    updateState();
    m_resetSerial = m_serial;
}

void QWaylandTextInput::text_input_leave()
{
    if (!m_commit.isEmpty())
        text_input_commit_string(0, m_commit);
}

void QWaylandTextInput::text_input_keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(modifiers);
    if (!QGuiApplication::focusObject())
        return;

    // TODO: Convert modifiers to Qt::KeyboardModifiers.
    QKeyEvent event(toQEventType(state), toQtKey(sym), Qt::NoModifier);
    QCoreApplication::sendEvent(qGuiApp->focusWindow(), &event);
}

QWaylandInputContext::QWaylandInputContext(QWaylandDisplay *display)
    : QPlatformInputContext()
    , mDisplay(display)
    , mTextInput()
{
}

bool QWaylandInputContext::isValid() const
{
    return mDisplay->textInputManager() != 0;
}

void QWaylandInputContext::reset()
{
    if (!ensureTextInput())
        return;

    mTextInput->reset();
}

void QWaylandInputContext::commit()
{
    if (!ensureTextInput())
        return;

    if (!QGuiApplication::focusObject())
        return;

    QInputMethodEvent event;
    event.setCommitString(mTextInput->commitString());
    QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

    mTextInput->reset();
}

void QWaylandInputContext::update(Qt::InputMethodQueries queries)
{
    Q_UNUSED(queries);
    if (!ensureTextInput())
        return;

    mTextInput->updateState();
}

void QWaylandInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
    if (!ensureTextInput())
        return;

    mTextInput->invoke_action(0, cursorPosition); // FIXME button, to UTF8 cursor position
}

void QWaylandInputContext::showInputPanel()
{
    if (!ensureTextInput())
        return;

    mTextInput->show_input_panel();
}

void QWaylandInputContext::hideInputPanel()
{
    if (!ensureTextInput())
        return;

    mTextInput->hide_input_panel();
}

bool QWaylandInputContext::isInputPanelVisible() const
{
    return false;
}

void QWaylandInputContext::setFocusObject(QObject *object)
{
    if (!ensureTextInput())
        return;

    if (!object) {
        mTextInput->deactivate(mDisplay->defaultInputDevice()->wl_seat());
        return;
    }

    QWindow *window = QGuiApplication::focusWindow();
    if (!window || !window->handle())
        return;

    struct ::wl_surface *surface = static_cast<QWaylandWindow *>(window->handle())->object();
    mTextInput->activate(mDisplay->defaultInputDevice()->wl_seat(), surface);
}

bool QWaylandInputContext::ensureTextInput()
{
    if (mTextInput)
        return true;

    if (!isValid())
        return false;

    mTextInput.reset(new QWaylandTextInput(mDisplay->textInputManager()->create_text_input()));
    return true;
}

}

QT_END_NAMESPACE

