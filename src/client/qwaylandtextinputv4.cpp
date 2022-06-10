// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandtextinputv4_p.h"

#include "qwaylandwindow_p.h"
#include "qwaylandinputmethodeventbuilder_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qevent.h>
#include <QtGui/qwindow.h>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcQpaWaylandTextInput, "qt.qpa.wayland.textinput")

namespace QtWaylandClient {

QWaylandTextInputv4::QWaylandTextInputv4(QWaylandDisplay *display,
                                         struct ::zwp_text_input_v4 *text_input)
    : QtWayland::zwp_text_input_v4(text_input)
    , m_display(display)
{

}

QWaylandTextInputv4::~QWaylandTextInputv4()
{
}

namespace {
const Qt::InputMethodQueries supportedQueries = Qt::ImEnabled |
                                                Qt::ImSurroundingText |
                                                Qt::ImCursorPosition |
                                                Qt::ImAnchorPosition |
                                                Qt::ImHints |
                                                Qt::ImCursorRectangle;
}

void QWaylandTextInputv4::zwp_text_input_v4_enter(struct ::wl_surface *surface)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;

    m_surface = surface;

    m_pendingPreeditString.clear();
    m_pendingCommitString.clear();
    m_pendingDeleteBeforeText = 0;
    m_pendingDeleteAfterText = 0;

    enable();
    updateState(supportedQueries, update_state_enter);
}

void QWaylandTextInputv4::zwp_text_input_v4_leave(struct ::wl_surface *surface)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;

    if (m_surface != surface) {
        qCWarning(qLcQpaWaylandTextInput()) << Q_FUNC_INFO << "Got leave event for surface" << surface << "focused surface" << m_surface;
        return;
    }

    // QTBUG-97248: check commit_mode
    // Currently text-input-unstable-v4-wip is implemented with preedit_commit_mode
    // 'commit'

    m_currentPreeditString.clear();

    m_surface = nullptr;
    m_currentSerial = 0U;

    disable();
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "Done";
}

void QWaylandTextInputv4::zwp_text_input_v4_preedit_string(const QString &text, int32_t cursorBegin, int32_t cursorEnd)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << text << cursorBegin << cursorEnd;

    if (!QGuiApplication::focusObject())
        return;

    m_pendingPreeditString.text = text;
    m_pendingPreeditString.cursorBegin = cursorBegin;
    m_pendingPreeditString.cursorEnd = cursorEnd;
}

void QWaylandTextInputv4::zwp_text_input_v4_commit_string(const QString &text)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << text;

    if (!QGuiApplication::focusObject())
        return;

    m_pendingCommitString = text;
}

void QWaylandTextInputv4::zwp_text_input_v4_delete_surrounding_text(uint32_t beforeText, uint32_t afterText)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << beforeText << afterText;

    if (!QGuiApplication::focusObject())
        return;

    m_pendingDeleteBeforeText = QWaylandInputMethodEventBuilder::indexFromWayland(m_surroundingText, beforeText);
    m_pendingDeleteAfterText = QWaylandInputMethodEventBuilder::indexFromWayland(m_surroundingText, afterText);
}

void QWaylandTextInputv4::zwp_text_input_v4_done(uint32_t serial)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "with serial" << serial << m_currentSerial;

    // This is a case of double click.
    // text_input_v4 will ignore this done signal and just keep the selection of the clicked word.
    if (m_cursorPos != m_anchorPos && (m_pendingDeleteBeforeText != 0 || m_pendingDeleteAfterText != 0)) {
        qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "Ignore done";
        m_pendingDeleteBeforeText = 0;
        m_pendingDeleteAfterText = 0;
        m_pendingPreeditString.clear();
        m_pendingCommitString.clear();
        return;
    }

    QObject *focusObject = QGuiApplication::focusObject();
    if (!focusObject)
        return;

    if (!m_surface) {
        qCWarning(qLcQpaWaylandTextInput) << Q_FUNC_INFO << serial << "Surface is not enabled yet";
        return;
    }

    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "PREEDIT" << m_pendingPreeditString.text << m_pendingPreeditString.cursorBegin;

    QList<QInputMethodEvent::Attribute> attributes;
    {
        if (m_pendingPreeditString.cursorBegin != -1 ||
                m_pendingPreeditString.cursorEnd != -1) {
            // Current supported cursor shape is just line.
            // It means, cursorEnd and cursorBegin are the same.
            QInputMethodEvent::Attribute attribute1(QInputMethodEvent::Cursor,
                                                    m_pendingPreeditString.text.length(),
                                                    1);
            attributes.append(attribute1);
        }

        // only use single underline style for now
        QTextCharFormat format;
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        QInputMethodEvent::Attribute attribute2(QInputMethodEvent::TextFormat,
                                                0,
                                                m_pendingPreeditString.text.length(), format);
        attributes.append(attribute2);
    }
    QInputMethodEvent event(m_pendingPreeditString.text, attributes);

    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "DELETE" << m_pendingDeleteBeforeText << m_pendingDeleteAfterText;
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "COMMIT" << m_pendingCommitString;

    // A workaround for reselection
    // It will disable redundant commit after reselection
    if (m_pendingDeleteBeforeText != 0 || m_pendingDeleteAfterText != 0)
        m_condReselection = true;

    event.setCommitString(m_pendingCommitString,
                          -m_pendingDeleteBeforeText,
                          m_pendingDeleteBeforeText + m_pendingDeleteAfterText);
    m_currentPreeditString = m_pendingPreeditString;
    m_pendingPreeditString.clear();
    m_pendingCommitString.clear();
    m_pendingDeleteBeforeText = 0;
    m_pendingDeleteAfterText = 0;
    QCoreApplication::sendEvent(focusObject, &event);

    if (serial == m_currentSerial)
        updateState(supportedQueries, update_state_full);
}

void QWaylandTextInputv4::reset()
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;

    m_pendingPreeditString.clear();
}

void QWaylandTextInputv4::enableSurface(::wl_surface *)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;
}

void QWaylandTextInputv4::disableSurface(::wl_surface *surface)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;

    if (m_surface != surface) {
        qCWarning(qLcQpaWaylandTextInput()) << Q_FUNC_INFO << "for surface" << surface << "focused surface" << m_surface;
        return;
    }
}

void QWaylandTextInputv4::commit()
{
    m_currentSerial = (m_currentSerial < UINT_MAX) ? m_currentSerial + 1U: 0U;

    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << "with serial" << m_currentSerial;
    QtWayland::zwp_text_input_v4::commit();
}

void QWaylandTextInputv4::updateState(Qt::InputMethodQueries queries, uint32_t flags)
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO << queries << flags;

    if (!QGuiApplication::focusObject())
        return;

    if (!QGuiApplication::focusWindow() || !QGuiApplication::focusWindow()->handle())
        return;

    auto *window = static_cast<QWaylandWindow *>(QGuiApplication::focusWindow()->handle());
    auto *surface = window->wlSurface();
    if (!surface || (surface != m_surface))
        return;

    queries &= supportedQueries;
    bool needsCommit = false;

    QInputMethodQueryEvent event(queries);
    QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

    // For some reason, a query for Qt::ImSurroundingText gives an empty string even though it is not.
    if (!(queries & Qt::ImSurroundingText) && event.value(Qt::ImSurroundingText).toString().isEmpty()) {
        return;
    }

    if (queries & Qt::ImCursorRectangle) {
        const QRect &cRect = event.value(Qt::ImCursorRectangle).toRect();
        const QRect &windowRect = QGuiApplication::inputMethod()->inputItemTransform().mapRect(cRect);
        const QMargins margins = window->frameMargins();
        const QRect &surfaceRect = windowRect.translated(margins.left(), margins.top());
        if (surfaceRect != m_cursorRect) {
            set_cursor_rectangle(surfaceRect.x(), surfaceRect.y(), surfaceRect.width(), surfaceRect.height());
            m_cursorRect = surfaceRect;
            needsCommit = true;
        }
    }

    if ((queries & Qt::ImSurroundingText) || (queries & Qt::ImCursorPosition) || (queries & Qt::ImAnchorPosition)) {
        QString text = event.value(Qt::ImSurroundingText).toString();
        int cursor = event.value(Qt::ImCursorPosition).toInt();
        int anchor = event.value(Qt::ImAnchorPosition).toInt();

        qCDebug(qLcQpaWaylandTextInput) << "Orginal surrounding_text from InputMethodQuery: " << text << cursor << anchor;

        // Make sure text is not too big
        // surround_text cannot exceed 4000byte in wayland protocol
        // The worst case will be supposed here.
        const int MAX_MESSAGE_SIZE = 4000;

        if (text.toUtf8().size() > MAX_MESSAGE_SIZE) {
            const int selectionStart = QWaylandInputMethodEventBuilder::indexToWayland(text, qMin(cursor, anchor));
            const int selectionEnd = QWaylandInputMethodEventBuilder::indexToWayland(text, qMax(cursor, anchor));
            const int selectionLength = selectionEnd - selectionStart;
            // If selection is bigger than 4000 byte, it is fixed to 4000 byte.
            // anchor will be moved in the 4000 byte boundary.
            if (selectionLength > MAX_MESSAGE_SIZE) {
                if (anchor > cursor) {
                    const int length = MAX_MESSAGE_SIZE;
                    anchor = QWaylandInputMethodEventBuilder::trimmedIndexFromWayland(text, length, cursor);
                    anchor -= cursor;
                    text = text.mid(cursor, anchor);
                    cursor = 0;
                } else {
                    const int length = -MAX_MESSAGE_SIZE;
                    anchor = QWaylandInputMethodEventBuilder::trimmedIndexFromWayland(text, length, cursor);
                    cursor -= anchor;
                    text = text.mid(anchor, cursor);
                    anchor = 0;
                }
            } else {
                const int offset = (MAX_MESSAGE_SIZE - selectionLength) / 2;

                int textStart = QWaylandInputMethodEventBuilder::trimmedIndexFromWayland(text, -offset, qMin(cursor, anchor));
                int textEnd = QWaylandInputMethodEventBuilder::trimmedIndexFromWayland(text, MAX_MESSAGE_SIZE, textStart);

                anchor -= textStart;
                cursor -= textStart;
                text = text.mid(textStart, textEnd - textStart);
            }
        }
        qCDebug(qLcQpaWaylandTextInput) << "Modified surrounding_text: " << text << cursor << anchor;

        const int cursorPos = QWaylandInputMethodEventBuilder::indexToWayland(text, cursor);
        const int anchorPos = QWaylandInputMethodEventBuilder::indexToWayland(text, anchor);

        if (m_surroundingText != text || m_cursorPos != cursorPos || m_anchorPos != anchorPos) {
            qCDebug(qLcQpaWaylandTextInput) << "Current surrounding_text: " << m_surroundingText << m_cursorPos << m_anchorPos;
            qCDebug(qLcQpaWaylandTextInput) << "New surrounding_text: " << text << cursorPos << anchorPos;

            set_surrounding_text(text, cursorPos, anchorPos);

            // A workaround in the case of reselection
            // It will work when re-clicking a preedit text
            if (m_condReselection) {
                qCDebug(qLcQpaWaylandTextInput) << "\"commit\" is disabled when Reselection by changing focus";
                m_condReselection = false;
                needsCommit = false;

            }

            m_surroundingText = text;
            m_cursorPos = cursorPos;
            m_anchorPos = anchorPos;
            m_cursor = cursor;
        }
    }

    if (queries & Qt::ImHints) {
        QWaylandInputMethodContentType contentType = QWaylandInputMethodContentType::convertV4(static_cast<Qt::InputMethodHints>(event.value(Qt::ImHints).toInt()));
        qCDebug(qLcQpaWaylandTextInput) << m_contentHint << contentType.hint;
        qCDebug(qLcQpaWaylandTextInput) << m_contentPurpose << contentType.purpose;

        if (m_contentHint != contentType.hint || m_contentPurpose != contentType.purpose) {
            qCDebug(qLcQpaWaylandTextInput) << "set_content_type: " << contentType.hint << contentType.purpose;
            set_content_type(contentType.hint, contentType.purpose);

            m_contentHint = contentType.hint;
            m_contentPurpose = contentType.purpose;
            needsCommit = true;
        }
    }

    if (needsCommit
            && (flags == update_state_change || flags == update_state_enter))
        commit();
}

void QWaylandTextInputv4::setCursorInsidePreedit(int cursor)
{
    Q_UNUSED(cursor);
    qCWarning(qLcQpaWaylandTextInput) << "QWaylandTextInputV4: Input protocol \"text-input-unstable-v4-wip\" does not support setting cursor inside preedit. Use qt-text-input-method-unstable-v1 instead for full support of Qt input method events.";
}

bool QWaylandTextInputv4::isInputPanelVisible() const
{
    qCWarning(qLcQpaWaylandTextInput) << "QWaylandTextInputV4: Input protocol \"text-input-unstable-v4-wip\" does not support querying input method visibility. Use qt-text-input-method-unstable-v1 instead for full support of Qt input method events.";
    return false;
}

QRectF QWaylandTextInputv4::keyboardRect() const
{
    qCDebug(qLcQpaWaylandTextInput) << Q_FUNC_INFO;
    return m_cursorRect;
}

QLocale QWaylandTextInputv4::locale() const
{
    qCWarning(qLcQpaWaylandTextInput) << "QWaylandTextInputV4: Input protocol \"text-input-unstable-v4-wip\" does not support querying input language. Use qt-text-input-method-unstable-v1 instead for full support of Qt input method events.";
    return QLocale();
}

Qt::LayoutDirection QWaylandTextInputv4::inputDirection() const
{
    qCWarning(qLcQpaWaylandTextInput) << "QWaylandTextInputV4: Input protocol \"text-input-unstable-v4-wip\" does not support querying input direction. Use qt-text-input-method-unstable-v1 instead for full support of Qt input method events.";
    return Qt::LeftToRight;
}

}

QT_END_NAMESPACE
