// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandinputmethodcontrol.h"
#include "qwaylandinputmethodcontrol_p.h"

#include "qwaylandcompositor.h"
#include "qwaylandseat.h"
#include "qwaylandsurface.h"
#include "qwaylandview.h"
#include "qwaylandtextinput.h"
#include "qwaylandtextinputv3.h"
#include "qwaylandqttextinputmethod.h"

#include <QtGui/QInputMethodEvent>

QWaylandInputMethodControl::QWaylandInputMethodControl(QWaylandSurface *surface)
    : QObject(*new QWaylandInputMethodControlPrivate(surface), surface)
{
    connect(d_func()->compositor, &QWaylandCompositor::defaultSeatChanged,
            this, &QWaylandInputMethodControl::defaultSeatChanged);

    updateTextInput();

    QWaylandTextInputV3 *textInputV3 = d_func()->textInputV3();
    if (textInputV3) {
        connect(textInputV3, &QWaylandTextInputV3::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled);
        connect(textInputV3, &QWaylandTextInputV3::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled);
        connect(textInputV3, &QWaylandTextInputV3::updateInputMethod, this, &QWaylandInputMethodControl::updateInputMethod);
    }

    QWaylandQtTextInputMethod *textInputMethod = d_func()->textInputMethod();
    if (textInputMethod) {
        connect(textInputMethod, &QWaylandQtTextInputMethod::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled);
        connect(textInputMethod, &QWaylandQtTextInputMethod::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled);
        connect(textInputMethod, &QWaylandQtTextInputMethod::updateInputMethod, this, &QWaylandInputMethodControl::updateInputMethod);
    }
}

QVariant QWaylandInputMethodControl::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
    Q_D(const QWaylandInputMethodControl);

    QWaylandTextInput *textInput = d->textInput();
    if (textInput != nullptr && textInput->focus() == d->surface)
        return textInput->inputMethodQuery(query, argument);

    QWaylandTextInputV3 *textInputV3 = d->textInputV3();
    if (textInputV3 != nullptr && textInputV3->focus() == d->surface)
        return textInputV3->inputMethodQuery(query, argument);

    QWaylandQtTextInputMethod *textInputMethod = d_func()->textInputMethod();
    if (textInputMethod && textInputMethod->focusedSurface() == d->surface)
        return textInputMethod->inputMethodQuery(query, argument);

    return QVariant();
}

void QWaylandInputMethodControl::inputMethodEvent(QInputMethodEvent *event)
{
    Q_D(QWaylandInputMethodControl);

    if (QWaylandTextInput *textInput = d->textInput()) {
        textInput->sendInputMethodEvent(event);
    } else if (QWaylandTextInputV3 *textInputV3 = d->textInputV3()) {
        textInputV3->sendInputMethodEvent(event);
    } else if (QWaylandQtTextInputMethod *textInputMethod = d->textInputMethod()) {
        textInputMethod->sendInputMethodEvent(event);
    } else {
        event->ignore();
    }
}

bool QWaylandInputMethodControl::enabled() const
{
    Q_D(const QWaylandInputMethodControl);

    return d->enabled;
}

void QWaylandInputMethodControl::setEnabled(bool enabled)
{
    Q_D(QWaylandInputMethodControl);

    if (d->enabled == enabled)
        return;

    d->enabled = enabled;
    emit enabledChanged(enabled);
    emit updateInputMethod(Qt::ImQueryInput);
}

void QWaylandInputMethodControl::surfaceEnabled(QWaylandSurface *surface)
{
    Q_D(QWaylandInputMethodControl);

    if (surface == d->surface)
        setEnabled(true);
}

void QWaylandInputMethodControl::surfaceDisabled(QWaylandSurface *surface)
{
    Q_D(QWaylandInputMethodControl);

    if (surface == d->surface)
        setEnabled(false);
}

void QWaylandInputMethodControl::setSurface(QWaylandSurface *surface)
{
    Q_D(QWaylandInputMethodControl);

    if (d->surface == surface)
        return;

    d->surface = surface;

    QWaylandTextInput *textInput = d->textInput();
    QWaylandTextInputV3 *textInputV3 = d->textInputV3();
    QWaylandQtTextInputMethod *textInputMethod = d->textInputMethod();
    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
               || (textInputV3 && textInputV3->isSurfaceEnabled(d->surface))
               || (textInputMethod && textInputMethod->isSurfaceEnabled(d->surface)));
}

void QWaylandInputMethodControl::updateTextInput()
{
    QWaylandTextInput *textInput = d_func()->textInput();

    if (textInput) {
        connect(textInput, &QWaylandTextInput::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled, Qt::UniqueConnection);
        connect(textInput, &QWaylandTextInput::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled, Qt::UniqueConnection);
        connect(textInput, &QWaylandTextInput::updateInputMethod, this, &QWaylandInputMethodControl::updateInputMethod, Qt::UniqueConnection);
    }
}

void QWaylandInputMethodControl::defaultSeatChanged()
{
    Q_D(QWaylandInputMethodControl);

    disconnect(d->textInput(), nullptr, this, nullptr);
    disconnect(d->textInputV3(), nullptr, this, nullptr);
    disconnect(d->textInputMethod(), nullptr, this, nullptr);

    d->seat = d->compositor->defaultSeat();
    QWaylandTextInput *textInput = d->textInput();
    QWaylandTextInputV3 *textInputV3 = d->textInputV3();
    QWaylandQtTextInputMethod *textInputMethod = d->textInputMethod();

    if (textInput) {
        connect(textInput, &QWaylandTextInput::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled);
        connect(textInput, &QWaylandTextInput::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled);
    }

    if (textInputV3) {
        connect(textInputV3, &QWaylandTextInputV3::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled);
        connect(textInputV3, &QWaylandTextInputV3::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled);
    }

    if (textInputMethod) {
        connect(textInputMethod, &QWaylandQtTextInputMethod::surfaceEnabled, this, &QWaylandInputMethodControl::surfaceEnabled);
        connect(textInputMethod, &QWaylandQtTextInputMethod::surfaceDisabled, this, &QWaylandInputMethodControl::surfaceDisabled);
    }

    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
               || (textInputV3 && textInputV3->isSurfaceEnabled(d->surface))
               || (textInputMethod && textInputMethod->isSurfaceEnabled(d->surface)));
}

QWaylandInputMethodControlPrivate::QWaylandInputMethodControlPrivate(QWaylandSurface *surface)
    : compositor(surface->compositor())
    , seat(compositor->defaultSeat())
    , surface(surface)
{
}

QWaylandQtTextInputMethod *QWaylandInputMethodControlPrivate::textInputMethod() const
{
    if (!surface->client() || !surface->client()->textInputProtocols().testFlag(QWaylandClient::TextInputProtocol::QtTextInputMethodV1))
        return nullptr;
    return QWaylandQtTextInputMethod::findIn(seat);
}

QWaylandTextInput *QWaylandInputMethodControlPrivate::textInput() const
{
    if (!surface->client() || !surface->client()->textInputProtocols().testFlag(QWaylandClient::TextInputProtocol::TextInputV2))
        return nullptr;
    return QWaylandTextInput::findIn(seat);
}

QWaylandTextInputV3 *QWaylandInputMethodControlPrivate::textInputV3() const
{
    return QWaylandTextInputV3::findIn(seat);
}

#include "moc_qwaylandinputmethodcontrol.cpp"
