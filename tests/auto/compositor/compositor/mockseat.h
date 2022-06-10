// Copyright (C) 2016 LG Electronics Ltd., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef MOCKSEAT
#define MOCKSEAT

#include "mockpointer.h"
#include "mockkeyboard.h"

#include <QObject>
#include "wayland-wayland-client-protocol.h"

class MockSeat : public QObject
{
    Q_OBJECT

public:
    MockSeat(wl_seat *seat);
    ~MockSeat() override;
    MockPointer *pointer() const { return m_pointer.data(); }
    MockKeyboard *keyboard() const { return m_keyboard.data(); }

    wl_seat *m_seat = nullptr;

private:
    QScopedPointer<MockPointer> m_pointer;
    QScopedPointer<MockKeyboard> m_keyboard;
};

#endif
