// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testkeyboardgrabber.h"

TestKeyboardGrabber::TestKeyboardGrabber(QWaylandSeat *seat)
    : QWaylandKeyboard(seat)
{
}

void TestKeyboardGrabber::setFocus(QWaylandSurface *surface)
{
    Q_EMIT focusedCalled();
    QWaylandKeyboard::setFocus(surface);
}

void TestKeyboardGrabber::sendKeyPressEvent(uint code)
{
    Q_EMIT keyPressCalled();
    QWaylandKeyboard::sendKeyPressEvent(code);
}

void TestKeyboardGrabber::sendKeyReleaseEvent(uint code)
{
    Q_EMIT keyReleaseCalled();
    QWaylandKeyboard::sendKeyReleaseEvent(code);
}

void TestKeyboardGrabber::sendKeyModifiers(QWaylandClient *client, uint32_t serial)
{
    Q_EMIT modifiersCalled();
    QWaylandKeyboard::sendKeyModifiers(client, serial);
}
