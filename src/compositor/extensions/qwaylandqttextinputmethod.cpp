// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandqttextinputmethod.h"
#include "qwaylandqttextinputmethod_p.h"

#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qinputmethod.h>
#include <QtGui/qcolor.h>
#include <QtGui/qtextformat.h>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/qwaylandsurface.h>

QT_BEGIN_NAMESPACE

QWaylandQtTextInputMethodPrivate::QWaylandQtTextInputMethodPrivate(QWaylandCompositor *c)
    : compositor(c)
{
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_enable(Resource *resource, struct ::wl_resource *surface)
{
    Q_Q(QWaylandQtTextInputMethod);
    if (this->resource == resource) {
        QWaylandSurface *waylandSurface = QWaylandSurface::fromResource(surface);
        if (surface != nullptr) {
            enabledSurfaces[resource] = waylandSurface;
            emit q->surfaceEnabled(waylandSurface);
        }
    }
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_disable(Resource *resource, struct ::wl_resource *surface)
{
    Q_Q(QWaylandQtTextInputMethod);
    if (this->resource == resource) {
        QWaylandSurface *waylandSurface = QWaylandSurface::fromResource(surface);
        QWaylandSurface *enabledSurface = enabledSurfaces.take(resource);

        if (Q_UNLIKELY(enabledSurface != waylandSurface))
            qCWarning(qLcWaylandCompositorInputMethods) << "Disabled surface does not match the one currently enabled";

        emit q->surfaceEnabled(waylandSurface);
    }
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_destroy(Resource *resource)
{
    if (this->resource == resource)
        wl_resource_destroy(resource->handle);
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_reset(Resource *resource)
{
    if (this->resource == resource)
        qApp->inputMethod()->reset();
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_commit(Resource *resource)
{
    if (this->resource == resource)
        qApp->inputMethod()->commit();
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_show_input_panel(Resource *resource)
{
    if (this->resource == resource)
        qApp->inputMethod()->show();
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_hide_input_panel(Resource *resource)
{
    if (this->resource == resource)
        qApp->inputMethod()->hide();
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_invoke_action(Resource *resource, int32_t type, int32_t cursorPosition)
{
    if (this->resource == resource)
        qApp->inputMethod()->invokeAction(QInputMethod::Action(type), cursorPosition);
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (this->resource == resource)
        cursorRectangle = QRect(x, y, width, height);
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_start_update(Resource *resource, int32_t queries)
{
    if (this->resource == resource)
        updatingQueries = Qt::InputMethodQueries(queries);
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_hints(Resource *resource, int32_t hints)
{
    if (this->resource == resource)
        this->hints = Qt::InputMethodHints(hints);
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_anchor_position(Resource *resource, int32_t anchorPosition)
{
    if (this->resource == resource)
        this->anchorPosition = anchorPosition;
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_cursor_position(Resource *resource, int32_t cursorPosition)
{
    if (this->resource == resource)
        this->cursorPosition = cursorPosition;
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_surrounding_text(Resource *resource, const QString &surroundingText, int32_t surroundingTextOffset)
{
    if (this->resource == resource) {
        this->surroundingText = surroundingText;
        this->surroundingTextOffset = surroundingTextOffset;
    }
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_absolute_position(Resource *resource, int32_t absolutePosition)
{
    if (this->resource == resource)
        this->absolutePosition = absolutePosition;
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_update_preferred_language(Resource *resource, const QString &preferredLanguage)
{
    if (this->resource == resource)
        this->preferredLanguage = preferredLanguage;
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_end_update(Resource *resource)
{
    Q_Q(QWaylandQtTextInputMethod);
    if (this->resource == resource && updatingQueries != 0) {
        Qt::InputMethodQueries queries = updatingQueries;
        updatingQueries = Qt::InputMethodQueries();
        emit q->updateInputMethod(queries);
    }
}

void QWaylandQtTextInputMethodPrivate::text_input_method_v1_acknowledge_input_method(Resource *resource)
{
    if (this->resource == resource)
        waitingForSync = false;
}

QWaylandQtTextInputMethod::QWaylandQtTextInputMethod(QWaylandObject *container, QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(container, *new QWaylandQtTextInputMethodPrivate(compositor))
{
    connect(&d_func()->focusDestroyListener, &QWaylandDestroyListener::fired,
            this, &QWaylandQtTextInputMethod::focusSurfaceDestroyed);

    connect(qGuiApp->inputMethod(), &QInputMethod::visibleChanged, this, &QWaylandQtTextInputMethod::sendVisibleChanged);
    connect(qGuiApp->inputMethod(), &QInputMethod::keyboardRectangleChanged, this, &QWaylandQtTextInputMethod::sendKeyboardRectangleChanged);
    connect(qGuiApp->inputMethod(), &QInputMethod::inputDirectionChanged, this, &QWaylandQtTextInputMethod::sendInputDirectionChanged);
    connect(qGuiApp->inputMethod(), &QInputMethod::localeChanged, this, &QWaylandQtTextInputMethod::sendLocaleChanged);
}


QWaylandQtTextInputMethod::~QWaylandQtTextInputMethod()
{
}

void QWaylandQtTextInputMethod::focusSurfaceDestroyed()
{
    Q_D(QWaylandQtTextInputMethod);
    d->focusDestroyListener.reset();
    d->waitingForSync = false;
    d->resource = nullptr;
    d->focusedSurface = nullptr;
}

QWaylandSurface *QWaylandQtTextInputMethod::focusedSurface() const
{
    Q_D(const QWaylandQtTextInputMethod);
    return d->focusedSurface;
}

QVariant QWaylandQtTextInputMethod::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    Q_D(const QWaylandQtTextInputMethod);
    switch (property) {
    case Qt::ImHints:
        return int(d->hints);
    case Qt::ImCursorRectangle:
        return d->cursorRectangle;
    case Qt::ImCursorPosition:
        return d->cursorPosition;
    case Qt::ImSurroundingText:
        return d->surroundingText;
    case Qt::ImAbsolutePosition:
        return d->absolutePosition;
    case Qt::ImCurrentSelection:
        return d->surroundingText.mid(qMin(d->cursorPosition, d->anchorPosition),
                                      qAbs(d->anchorPosition - d->cursorPosition));
    case Qt::ImAnchorPosition:
        return d->anchorPosition;
    case Qt::ImTextAfterCursor:
        if (argument.isValid())
            return d->surroundingText.mid(d->cursorPosition, argument.toInt());
        return d->surroundingText.mid(d->cursorPosition);
    case Qt::ImTextBeforeCursor:
        if (argument.isValid())
            return d->surroundingText.left(d->cursorPosition).right(argument.toInt());
        return d->surroundingText.left(d->cursorPosition);
    case Qt::ImPreferredLanguage:
        return d->preferredLanguage;

    default:
        return QVariant();
    }
}

void QWaylandQtTextInputMethod::sendKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr)
        return;

    d->send_key(d->resource->handle,
                int(event->type()),
                event->key(),
                event->modifiers(),
                event->isAutoRepeat(),
                event->count(),
                event->nativeScanCode(),
                event->nativeVirtualKey(),
                event->nativeModifiers(),
                event->text());
}

void QWaylandQtTextInputMethod::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr || d->compositor == nullptr)
        return;

    if (d->updatingQueries != 0) {
        qCWarning(qLcWaylandCompositorInputMethods) << "Input method event sent while client is updating. Ignored.";
        return;
    }

    Q_ASSERT(!d->waitingForSync);

    QString oldSurroundText = d->surroundingText;
    int oldCursorPosition = d->cursorPosition;
    int oldAnchorPosition = d->anchorPosition;
    int oldAbsolutePosition = d->absolutePosition;
    QRect oldCursorRectangle = d->cursorRectangle;
    QString oldPreferredLanguage = d->preferredLanguage;
    Qt::InputMethodHints oldHints = d->hints;

    uint serial = d->compositor->nextSerial(); // ### Not needed if we block on this?
    d->send_start_input_method_event(d->resource->handle, serial, d->surroundingTextOffset);
    for (const QInputMethodEvent::Attribute &attribute : event->attributes()) {
        switch (attribute.type) {
        case QInputMethodEvent::TextFormat:
        {
            auto properties = attribute.value.value<QTextFormat>().properties();
            if (properties.size() != 2 || properties.firstKey() != QTextFormat::FontUnderline || properties.lastKey() != QTextFormat::TextUnderlineStyle) {
                qCWarning(qLcWaylandCompositorInputMethods()) << "Only underline text formats currently supported";
            }

            d->send_input_method_event_attribute(d->resource->handle,
                                                 serial,
                                                 attribute.type,
                                                 attribute.start,
                                                 attribute.length,
                                                 QString());
            break;
        }
        case QInputMethodEvent::Cursor:
            d->cursorPosition = attribute.start;
            d->send_input_method_event_attribute(d->resource->handle,
                                                 serial,
                                                 attribute.type,
                                                 attribute.start,
                                                 attribute.length,
                                                 attribute.value.typeId() == QMetaType::QColor ? attribute.value.value<QColor>().name() : QString());
            break;
        case QInputMethodEvent::Language: // ### What is the type of value? Is it string?
            Q_FALLTHROUGH();
        case QInputMethodEvent::Ruby:
            d->send_input_method_event_attribute(d->resource->handle,
                                                 serial,
                                                 attribute.type,
                                                 attribute.start,
                                                 attribute.length,
                                                 attribute.value.toString());
            break;
        case QInputMethodEvent::Selection:
            d->send_input_method_event_attribute(d->resource->handle,
                                                 serial,
                                                 attribute.type,
                                                 attribute.start,
                                                 attribute.length,
                                                 QString());
            break;
        }
    }

    d->waitingForSync = true;
    d->send_end_input_method_event(d->resource->handle,
                                   serial,
                                   event->commitString(),
                                   event->preeditString(),
                                   event->replacementStart(),
                                   event->replacementLength());

    while (d->waitingForSync)
        d->compositor->processWaylandEvents();

    Qt::InputMethodQueries queries;
    if (d->surroundingText != oldSurroundText)
        queries |= Qt::ImSurroundingText;
    if (d->cursorPosition != oldCursorPosition)
        queries |= Qt::ImCursorPosition;
    if (d->anchorPosition != oldAnchorPosition)
        queries |= Qt::ImAnchorPosition;
    if (d->absolutePosition != oldAbsolutePosition)
        queries |= Qt::ImAbsolutePosition;
    if (d->cursorRectangle != oldCursorRectangle)
        queries |= Qt::ImCursorRectangle;
    if (d->preferredLanguage != oldPreferredLanguage)
        queries |= Qt::ImPreferredLanguage;
    if (d->hints != oldHints)
        queries |= Qt::ImHints;
    if (queries != 0)
        emit updateInputMethod(queries);
}

bool QWaylandQtTextInputMethod::isSurfaceEnabled(QWaylandSurface *surface) const
{
    Q_D(const QWaylandQtTextInputMethod);
    return d->enabledSurfaces.values().contains(surface);
}

void QWaylandQtTextInputMethod::setFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandQtTextInputMethod);

    QWaylandQtTextInputMethodPrivate::Resource *resource = surface != nullptr ? d->resourceMap().value(surface->waylandClient()) : nullptr;
    if (d->resource == resource)
        return;

    if (d->resource != nullptr && d->focusedSurface != nullptr) {
        d->send_leave(d->resource->handle, d->focusedSurface->resource());
        d->focusDestroyListener.reset();
    }

    d->resource = resource;
    d->focusedSurface = surface;

    if (d->resource != nullptr && d->focusedSurface != nullptr) {
        d->surroundingText.clear();
        d->cursorPosition = 0;
        d->anchorPosition = 0;
        d->absolutePosition = 0;
        d->cursorRectangle = QRect();
        d->preferredLanguage.clear();
        d->hints = Qt::InputMethodHints();
        d->send_enter(d->resource->handle, d->focusedSurface->resource());
        sendInputDirectionChanged();
        sendLocaleChanged();
        sendInputDirectionChanged();
        d->focusDestroyListener.listenForDestruction(surface->resource());
        if (d->inputPanelVisible && d->enabledSurfaces.values().contains(surface))
            qGuiApp->inputMethod()->show();
    }
}

void QWaylandQtTextInputMethod::sendLocaleChanged()
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr)
        return;

    d->send_locale_changed(d->resource->handle, qGuiApp->inputMethod()->locale().bcp47Name());
}

void QWaylandQtTextInputMethod::sendInputDirectionChanged()
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr)
        return;

    d->send_input_direction_changed(d->resource->handle, int(qGuiApp->inputMethod()->inputDirection()));
}

void QWaylandQtTextInputMethod::sendKeyboardRectangleChanged()
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr)
        return;

    QRectF keyboardRectangle = qGuiApp->inputMethod()->keyboardRectangle();
    d->send_keyboard_rectangle_changed(d->resource->handle,
                                       wl_fixed_from_double(keyboardRectangle.x()),
                                       wl_fixed_from_double(keyboardRectangle.y()),
                                       wl_fixed_from_double(keyboardRectangle.width()),
                                       wl_fixed_from_double(keyboardRectangle.height()));
}

void QWaylandQtTextInputMethod::sendVisibleChanged()
{
    Q_D(QWaylandQtTextInputMethod);
    if (d->resource == nullptr || d->resource->handle == nullptr)
        return;

    d->send_visible_changed(d->resource->handle, int(qGuiApp->inputMethod()->isVisible()));
}

void QWaylandQtTextInputMethod::add(::wl_client *client, uint32_t id, int version)
{
    Q_D(QWaylandQtTextInputMethod);
    d->add(client, id, version);
}

const struct wl_interface *QWaylandQtTextInputMethod::interface()
{
    return QWaylandQtTextInputMethodPrivate::interface();
}

QByteArray QWaylandQtTextInputMethod::interfaceName()
{
    return QWaylandQtTextInputMethodPrivate::interfaceName();
}

QT_END_NAMESPACE

#include "moc_qwaylandqttextinputmethod.cpp"
