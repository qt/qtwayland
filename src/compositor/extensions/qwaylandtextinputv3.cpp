// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandtextinputv3.h"
#include "qwaylandtextinputv3_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/private/qwaylandseat_p.h>

#include "qwaylandsurface.h"
#include "qwaylandview.h"
#include "qwaylandinputmethodeventbuilder_p.h"

#include <QGuiApplication>
#include <QInputMethodEvent>
#include <qpa/qwindowsysteminterface.h>

#if QT_CONFIG(xkbcommon)
#include <QtGui/private/qxkbcommon_p.h>
#endif

QT_BEGIN_NAMESPACE

QWaylandTextInputV3ClientState::QWaylandTextInputV3ClientState()
{
}

Qt::InputMethodQueries QWaylandTextInputV3ClientState::updatedQueries(const QWaylandTextInputV3ClientState &other) const
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Qt::InputMethodQueries queries;

    if (hints != other.hints)
        queries |= Qt::ImHints;
    if (cursorRectangle != other.cursorRectangle)
        queries |= Qt::ImCursorRectangle;
    if (surroundingText != other.surroundingText)
        queries |= Qt::ImSurroundingText | Qt::ImCurrentSelection;
    if (cursorPosition != other.cursorPosition)
        queries |= Qt::ImCursorPosition | Qt::ImCurrentSelection;
    if (anchorPosition != other.anchorPosition)
        queries |= Qt::ImAnchorPosition | Qt::ImCurrentSelection;

    return queries;
}

Qt::InputMethodQueries QWaylandTextInputV3ClientState::mergeChanged(const QWaylandTextInputV3ClientState &other) {

    Qt::InputMethodQueries queries;

    if ((other.changedState & Qt::ImHints) && hints != other.hints) {
        hints = other.hints;
        queries |= Qt::ImHints;
    }

    if ((other.changedState & Qt::ImCursorRectangle) && cursorRectangle != other.cursorRectangle) {
        cursorRectangle = other.cursorRectangle;
        queries |= Qt::ImCursorRectangle;
    }

    if ((other.changedState & Qt::ImSurroundingText) && surroundingText != other.surroundingText) {
        surroundingText = other.surroundingText;
        queries |= Qt::ImSurroundingText | Qt::ImCurrentSelection;
    }

    if ((other.changedState & Qt::ImCursorPosition) && cursorPosition != other.cursorPosition) {
        cursorPosition = other.cursorPosition;
        queries |= Qt::ImCursorPosition | Qt::ImCurrentSelection;
    }

    if ((other.changedState & Qt::ImAnchorPosition) && anchorPosition != other.anchorPosition) {
        anchorPosition = other.anchorPosition;
        queries |= Qt::ImAnchorPosition | Qt::ImCurrentSelection;
    }

    return queries;
}

QWaylandTextInputV3Private::QWaylandTextInputV3Private(QWaylandCompositor *compositor)
    : compositor(compositor)
    , currentState(new QWaylandTextInputV3ClientState)
    , pendingState(new QWaylandTextInputV3ClientState)
{
}

void QWaylandTextInputV3Private::sendInputMethodEvent(QInputMethodEvent *event)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    if (!focusResource || !focusResource->handle)
        return;

    bool needsDone = false;

    const QString &newPreeditString = event->preeditString();

    // Current cursor shape is only line. It means both cursorBegin
    // and cursorEnd will be the same values.
    int32_t preeditCursorPos = newPreeditString.toUtf8().size();

    if (event->replacementLength() > 0) {
        int replacementStart = event->replacementStart();
        int replacementLength = event->replacementLength();
        const int cursorPos = currentState->cursorPosition;
        if (currentState->cursorPosition < -event->replacementStart()) {
            qCWarning(qLcWaylandCompositorTextInput)
                << Q_FUNC_INFO
                << "Invalid replacementStart :" << replacementStart
                << "on the cursorPosition :" << cursorPos;
            replacementStart = -cursorPos;
        }
        auto targetText = QStringView{currentState->surroundingText}.sliced(cursorPos + replacementStart);
        if (targetText.length() < replacementLength) {
            qCWarning(qLcWaylandCompositorTextInput)
                << Q_FUNC_INFO
                << "Invalid replacementLength :" << replacementLength
                << "for the surrounding text :" << targetText;
            replacementLength = targetText.length();
        }
        const int before = targetText.first(-replacementStart).toUtf8().size();
        const int after = targetText.first(replacementLength).toUtf8().size() - before;

        send_delete_surrounding_text(focusResource->handle, before, after);
        needsDone = true;

        // The commit will also be applied here
        currentState->surroundingText.replace(cursorPos + replacementStart,
                                              replacementLength,
                                              event->commitString());
        currentState->cursorPosition = cursorPos + replacementStart + event->commitString().length();
        currentState->anchorPosition = cursorPos + replacementStart + event->commitString().length();
        qApp->inputMethod()->update(Qt::ImSurroundingText | Qt::ImCursorPosition | Qt::ImAnchorPosition);
    }

    if (currentPreeditString != newPreeditString) {
        currentPreeditString = newPreeditString;
        send_preedit_string(focusResource->handle, currentPreeditString, preeditCursorPos, preeditCursorPos);
        needsDone = true;
    }
    if (!event->commitString().isEmpty()) {
        send_commit_string(focusResource->handle, event->commitString());
        needsDone = true;
    }

    if (needsDone)
        send_done(focusResource->handle, serials[focusResource]);
}


void QWaylandTextInputV3Private::sendKeyEvent(QKeyEvent *event)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    if (!focusResource || !focusResource->handle)
        return;

    send_commit_string(focusResource->handle, event->text());

    send_done(focusResource->handle, serials[focusResource]);
}

QVariant QWaylandTextInputV3Private::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << property;

    switch (property) {
    case Qt::ImHints:
        return QVariant(static_cast<int>(currentState->hints));
    case Qt::ImCursorRectangle:
        return currentState->cursorRectangle;
    case Qt::ImFont:
        // Not supported
        return QVariant();
    case Qt::ImCursorPosition:
        qCDebug(qLcWaylandCompositorTextInput) << currentState->cursorPosition;
        return currentState->cursorPosition;
    case Qt::ImSurroundingText:
        qCDebug(qLcWaylandCompositorTextInput) << currentState->surroundingText;
        return currentState->surroundingText;
    case Qt::ImCurrentSelection:
        return currentState->surroundingText.mid(qMin(currentState->cursorPosition, currentState->anchorPosition),
                                                 qAbs(currentState->anchorPosition - currentState->cursorPosition));
    case Qt::ImMaximumTextLength:
        // Not supported
        return QVariant();
    case Qt::ImAnchorPosition:
        qCDebug(qLcWaylandCompositorTextInput) << currentState->anchorPosition;
        return currentState->anchorPosition;
    case Qt::ImAbsolutePosition:
        // We assume the surrounding text is our whole document for now
        return currentState->cursorPosition;
    case Qt::ImTextAfterCursor:
        if (argument.isValid())
            return currentState->surroundingText.mid(currentState->cursorPosition, argument.toInt());
        return currentState->surroundingText.mid(currentState->cursorPosition);
    case Qt::ImTextBeforeCursor:
        if (argument.isValid())
            return currentState->surroundingText.left(currentState->cursorPosition).right(argument.toInt());
        return currentState->surroundingText.left(currentState->cursorPosition);

    default:
        return QVariant();
    }
}

void QWaylandTextInputV3Private::setFocus(QWaylandSurface *surface)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << surface;

    if (focusResource && focus) {
        qApp->inputMethod()->commit();
        qApp->inputMethod()->hide();
        inputPanelVisible = false;
        send_leave(focusResource->handle, focus->resource());
        currentPreeditString.clear();
    }

    if (focus != surface)
        focusDestroyListener.reset();

    Resource *resource = surface ? resourceMap().value(surface->waylandClient()) : 0;
    if (resource && surface) {
        send_enter(resource->handle, surface->resource());

        if (focus != surface)
            focusDestroyListener.listenForDestruction(surface->resource());
    }

    focus = surface;
    focusResource = resource;
}

void QWaylandTextInputV3Private::zwp_text_input_v3_bind_resource(Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    serials.insert(resource, 0);
}

void QWaylandTextInputV3Private::zwp_text_input_v3_destroy_resource(Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    serials.remove(resource);
    if (focusResource == resource)
        focusResource = nullptr;
}

void QWaylandTextInputV3Private::zwp_text_input_v3_destroy(Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void QWaylandTextInputV3Private::zwp_text_input_v3_enable(Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(QWaylandTextInputV3);

    pendingState.reset(new QWaylandTextInputV3ClientState);

    enabledSurfaces.insert(resource, focus);
    emit q->surfaceEnabled(focus);

    inputPanelVisible = true;
    qApp->inputMethod()->show();
}

void QWaylandTextInputV3Private::zwp_text_input_v3_disable(QtWaylandServer::zwp_text_input_v3::Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(QWaylandTextInputV3);

    QWaylandSurface *s = enabledSurfaces.take(resource);
    emit q->surfaceDisabled(s);

    // When reselecting a word by setFocus
    qApp->inputMethod()->commit();

    qApp->inputMethod()->reset();
    pendingState.reset(new QWaylandTextInputV3ClientState);
}

void QWaylandTextInputV3Private::zwp_text_input_v3_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << x << y << width << height;

    if (resource != focusResource)
        return;

    pendingState->cursorRectangle = QRect(x, y, width, height);

    pendingState->changedState |= Qt::ImCursorRectangle;
}

void QWaylandTextInputV3Private::zwp_text_input_v3_commit(Resource *resource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    serials[resource] = serials[resource] < UINT_MAX ? serials[resource] + 1U : 0U;

    // Just increase serials and ignore empty commits
    if (!pendingState->changedState) {
        qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << "pendingState is not changed";
        return;
    }

    // Selection starts.
    // But since qtvirtualkeyboard with hunspell does not reset its preedit string,
    // compositor forces to reset inputMethod.
    if ((currentState->cursorPosition == currentState->anchorPosition)
            && (pendingState->cursorPosition != pendingState->anchorPosition))
        qApp->inputMethod()->reset();

    // Enable reselection
    // This is a workaround to make qtvirtualkeyboad's state empty by clearing State::InputMethodClick.
    if (currentState->surroundingText == pendingState->surroundingText && currentState->cursorPosition != pendingState->cursorPosition)
        qApp->inputMethod()->invokeAction(QInputMethod::Click, pendingState->cursorPosition);

    Qt::InputMethodQueries queries = currentState->mergeChanged(*pendingState.data());
    pendingState.reset(new QWaylandTextInputV3ClientState);

    if (queries) {
        qCDebug(qLcWaylandCompositorTextInput) << "QInputMethod::update() after commit with" << queries;

        qApp->inputMethod()->update(queries);
    }
}

void QWaylandTextInputV3Private::zwp_text_input_v3_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << hint << purpose;

    if (resource != focusResource)
        return;

    pendingState->hints = Qt::ImhNone;

    if ((hint & content_hint_completion) == 0)
        pendingState->hints |= Qt::ImhNoPredictiveText;
    if ((hint & content_hint_auto_capitalization) == 0)
        pendingState->hints |= Qt::ImhNoAutoUppercase;
    if ((hint & content_hint_lowercase) != 0)
        pendingState->hints |= Qt::ImhPreferLowercase;
    if ((hint & content_hint_uppercase) != 0)
        pendingState->hints |= Qt::ImhPreferUppercase;
    if ((hint & content_hint_hidden_text) != 0)
        pendingState->hints |= Qt::ImhHiddenText;
    if ((hint & content_hint_sensitive_data) != 0)
        pendingState->hints |= Qt::ImhSensitiveData;
    if ((hint & content_hint_latin) != 0)
        pendingState->hints |= Qt::ImhLatinOnly;
    if ((hint & content_hint_multiline) != 0)
        pendingState->hints |= Qt::ImhMultiLine;

    switch (purpose) {
    case content_purpose_normal:
        break;
    case content_purpose_alpha:
        pendingState->hints |= Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly;
        break;
    case content_purpose_digits:
        pendingState->hints |= Qt::ImhDigitsOnly;
        break;
    case content_purpose_number:
        pendingState->hints |= Qt::ImhFormattedNumbersOnly;
        break;
    case content_purpose_phone:
        pendingState->hints |= Qt::ImhDialableCharactersOnly;
        break;
    case content_purpose_url:
        pendingState->hints |= Qt::ImhUrlCharactersOnly;
        break;
    case content_purpose_email:
        pendingState->hints |= Qt::ImhEmailCharactersOnly;
        break;
    case content_purpose_name:
    case content_purpose_password:
        break;
    case content_purpose_date:
        pendingState->hints |= Qt::ImhDate;
        break;
    case content_purpose_time:
        pendingState->hints |= Qt::ImhTime;
        break;
    case content_purpose_datetime:
        pendingState->hints |= Qt::ImhDate | Qt::ImhTime;
        break;
    case content_purpose_terminal:
    default:
        break;
    }

    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << pendingState->hints;

    pendingState->changedState |= Qt::ImHints;
}

void QWaylandTextInputV3Private::zwp_text_input_v3_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << text << cursor << anchor;

    if (resource != focusResource)
        return;

    pendingState->surroundingText = text;
    pendingState->cursorPosition = QWaylandInputMethodEventBuilder::indexFromWayland(text, cursor);
    pendingState->anchorPosition = QWaylandInputMethodEventBuilder::indexFromWayland(text, anchor);

    pendingState->changedState |= Qt::ImSurroundingText | Qt::ImCursorPosition | Qt::ImAnchorPosition;
}

void QWaylandTextInputV3Private::zwp_text_input_v3_set_text_change_cause(Resource *resource, uint32_t cause)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_UNUSED(resource);
    Q_UNUSED(cause);
}

QWaylandTextInputV3::QWaylandTextInputV3(QWaylandObject *container, QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(container, *new QWaylandTextInputV3Private(compositor))
{
    connect(&d_func()->focusDestroyListener, &QWaylandDestroyListener::fired,
            this, &QWaylandTextInputV3::focusSurfaceDestroyed);
}

QWaylandTextInputV3::~QWaylandTextInputV3()
{
}

void QWaylandTextInputV3::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_D(QWaylandTextInputV3);

    d->sendInputMethodEvent(event);
}

void QWaylandTextInputV3::sendKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandTextInputV3);

    d->sendKeyEvent(event);
}

QVariant QWaylandTextInputV3::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    const Q_D(QWaylandTextInputV3);

    return d->inputMethodQuery(property, argument);
}

QWaylandSurface *QWaylandTextInputV3::focus() const
{
    const Q_D(QWaylandTextInputV3);

    return d->focus;
}

void QWaylandTextInputV3::setFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandTextInputV3);

    d->setFocus(surface);
}

void QWaylandTextInputV3::focusSurfaceDestroyed(void *)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_D(QWaylandTextInputV3);

    d->focusDestroyListener.reset();

    d->focus = nullptr;
    d->focusResource = nullptr;
}

bool QWaylandTextInputV3::isSurfaceEnabled(QWaylandSurface *surface) const
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    const Q_D(QWaylandTextInputV3);

    return d->enabledSurfaces.values().contains(surface);
}

void QWaylandTextInputV3::add(::wl_client *client, uint32_t id, int version)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO << client << id << version;

    Q_D(QWaylandTextInputV3);

    d->add(client, id, version);
}

const wl_interface *QWaylandTextInputV3::interface()
{
    return QWaylandTextInputV3Private::interface();
}

QByteArray QWaylandTextInputV3::interfaceName()
{
    return QWaylandTextInputV3Private::interfaceName();
}

QT_END_NAMESPACE
