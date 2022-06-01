/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include <QLoggingCategory>
#include <QtGui/QGuiApplication>
#include <QtGui/QTextCharFormat>
#include <QtGui/QWindow>
#include <QtCore/QVarLengthArray>

#include "qwaylanddisplay_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandwindow_p.h"

#if QT_CONFIG(xkbcommon)
#include <locale.h>
#endif

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcQpaInputMethods, "qt.qpa.input.methods")

namespace QtWaylandClient {

QWaylandInputContext::QWaylandInputContext(QWaylandDisplay *display)
    : mDisplay(display)
{
}

QWaylandInputContext::~QWaylandInputContext()
{
}

bool QWaylandInputContext::isValid() const
{
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    return mDisplay->textInputManagerv2() != nullptr || mDisplay->textInputManagerv1() != nullptr || mDisplay->textInputManagerv4() != nullptr;
#else //  QT_WAYLAND_TEXT_INPUT_V4_WIP
    return mDisplay->textInputManagerv2() != nullptr || mDisplay->textInputManagerv1() != nullptr;
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
}

void QWaylandInputContext::reset()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;
#if QT_CONFIG(xkbcommon)
    if (m_composeState)
        xkb_compose_state_reset(m_composeState);
#endif

    QPlatformInputContext::reset();

    if (!textInput())
        return;

    textInput()->reset();
}

void QWaylandInputContext::commit()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return;

    textInput()->commit();
}

static ::wl_surface *surfaceForWindow(QWindow *window)
{
    if (!window || !window->handle())
        return nullptr;

    auto *waylandWindow = static_cast<QWaylandWindow *>(window->handle());
    return waylandWindow->wlSurface();
}

void QWaylandInputContext::update(Qt::InputMethodQueries queries)
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO << queries;

    if (!QGuiApplication::focusObject() || !textInput())
        return;

    auto *currentSurface = surfaceForWindow(mCurrentWindow);

    if (currentSurface && !inputMethodAccepted()) {
        textInput()->disableSurface(currentSurface);
        mCurrentWindow.clear();
    } else if (!currentSurface && inputMethodAccepted()) {
        QWindow *window = QGuiApplication::focusWindow();
        if (auto *focusSurface = surfaceForWindow(window)) {
            textInput()->enableSurface(focusSurface);
            mCurrentWindow = window;
        }
    }

    textInput()->updateState(queries, QWaylandTextInputInterface::update_state_change);
}

void QWaylandInputContext::invokeAction(QInputMethod::Action action, int cursorPostion)
{
    if (!textInput())
        return;

    if (action == QInputMethod::Click)
        textInput()->setCursorInsidePreedit(cursorPostion);
}

void QWaylandInputContext::showInputPanel()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return;

    textInput()->showInputPanel();
}

void QWaylandInputContext::hideInputPanel()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return;

    textInput()->hideInputPanel();
}

bool QWaylandInputContext::isInputPanelVisible() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return QPlatformInputContext::isInputPanelVisible();

    return textInput()->isInputPanelVisible();
}

QRectF QWaylandInputContext::keyboardRect() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return QPlatformInputContext::keyboardRect();

    return textInput()->keyboardRect();
}

QLocale QWaylandInputContext::locale() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return QPlatformInputContext::locale();

    return textInput()->locale();
}

Qt::LayoutDirection QWaylandInputContext::inputDirection() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput())
        return QPlatformInputContext::inputDirection();

    return textInput()->inputDirection();
}

void QWaylandInputContext::setFocusObject(QObject *object)
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;
#if QT_CONFIG(xkbcommon)
    m_focusObject = object;
#else
    Q_UNUSED(object);
#endif

    if (!textInput())
        return;

    QWindow *window = QGuiApplication::focusWindow();

    if (mCurrentWindow && mCurrentWindow->handle()) {
        if (mCurrentWindow.data() != window || !inputMethodAccepted()) {
            auto *surface = static_cast<QWaylandWindow *>(mCurrentWindow->handle())->wlSurface();
            if (surface)
                textInput()->disableSurface(surface);
            mCurrentWindow.clear();
        }
    }

    if (window && window->handle() && inputMethodAccepted()) {
        if (mCurrentWindow.data() != window) {
            auto *surface = static_cast<QWaylandWindow *>(window->handle())->wlSurface();
            if (surface) {
                textInput()->enableSurface(surface);
                mCurrentWindow = window;
            }
        }
        textInput()->updateState(Qt::ImQueryAll, QWaylandTextInputInterface::update_state_enter);
    }
}

QWaylandTextInputInterface *QWaylandInputContext::textInput() const
{
    return mDisplay->defaultInputDevice()->textInput();
}

#if QT_CONFIG(xkbcommon)

void QWaylandInputContext::ensureInitialized()
{
    if (m_initialized)
        return;

    if (!m_XkbContext) {
        qCWarning(qLcQpaInputMethods) << "error: xkb context has not been set on" << metaObject()->className();
        return;
    }

    m_initialized = true;
    const char *locale = setlocale(LC_CTYPE, "");
    if (!locale)
        locale = setlocale(LC_CTYPE, nullptr);
    qCDebug(qLcQpaInputMethods) << "detected locale (LC_CTYPE):" << locale;

    m_composeTable = xkb_compose_table_new_from_locale(m_XkbContext, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (m_composeTable)
        m_composeState = xkb_compose_state_new(m_composeTable, XKB_COMPOSE_STATE_NO_FLAGS);

    if (!m_composeTable) {
        qCWarning(qLcQpaInputMethods, "failed to create compose table");
        return;
    }
    if (!m_composeState) {
        qCWarning(qLcQpaInputMethods, "failed to create compose state");
        return;
    }
}

bool QWaylandInputContext::filterEvent(const QEvent *event)
{
    auto keyEvent = static_cast<const QKeyEvent *>(event);
    if (keyEvent->type() != QEvent::KeyPress)
        return false;

    if (!inputMethodAccepted())
        return false;

    // lazy initialization - we don't want to do this on an app startup
    ensureInitialized();

    if (!m_composeTable || !m_composeState)
        return false;

    xkb_compose_state_feed(m_composeState, keyEvent->nativeVirtualKey());

    switch (xkb_compose_state_get_status(m_composeState)) {
    case XKB_COMPOSE_COMPOSING:
        return true;
    case XKB_COMPOSE_CANCELLED:
        reset();
        return false;
    case XKB_COMPOSE_COMPOSED:
    {
        const int size = xkb_compose_state_get_utf8(m_composeState, nullptr, 0);
        QVarLengthArray<char, 32> buffer(size + 1);
        xkb_compose_state_get_utf8(m_composeState, buffer.data(), buffer.size());
        QString composedText = QString::fromUtf8(buffer.constData());

        QInputMethodEvent event;
        event.setCommitString(composedText);

        if (!m_focusObject && qApp)
            m_focusObject = qApp->focusObject();

        if (m_focusObject)
            QCoreApplication::sendEvent(m_focusObject, &event);
        else
            qCWarning(qLcQpaInputMethods, "no focus object");

        reset();
        return true;
    }
    case XKB_COMPOSE_NOTHING:
        return false;
    default:
        Q_UNREACHABLE();
        return false;
    }
}

#endif

}

QT_END_NAMESPACE

#include "moc_qwaylandinputcontext_p.cpp"
