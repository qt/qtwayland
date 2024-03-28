// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKKEYBOARD_H
#define MOCKKEYBOARD_H

#include <QObject>
#include "wayland-wayland-client-protocol.h"

class MockKeyboard : public QObject
{
    Q_OBJECT

public:
    explicit MockKeyboard(wl_seat *seat);
    ~MockKeyboard() override;

    wl_keyboard *m_keyboard = nullptr;
    wl_surface *m_enteredSurface = nullptr;
    uint m_lastKeyCode = 0;
    uint m_lastKeyState = 0;
    uint m_group = 0;
};

#endif // MOCKKEYBOARD_H
