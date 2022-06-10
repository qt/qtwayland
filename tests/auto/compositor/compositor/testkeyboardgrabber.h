// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qwaylandkeyboard.h"

class TestKeyboardGrabber : public QWaylandKeyboard
{
    Q_OBJECT
public:
    TestKeyboardGrabber(QWaylandSeat *seat);

    void setFocus(QWaylandSurface *surface) override;
    void sendKeyModifiers(QWaylandClient *client, uint32_t serial) override;
    void sendKeyPressEvent(uint code) override;
    void sendKeyReleaseEvent(uint code) override;

signals:
    void focusedCalled();
    void keyPressCalled();
    void keyReleaseCalled();
    void modifiersCalled();
};


