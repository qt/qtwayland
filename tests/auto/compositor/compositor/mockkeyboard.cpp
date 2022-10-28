// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mockkeyboard.h"

QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

void keyboardKeymap(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(format);
    Q_UNUSED(fd);
    Q_UNUSED(size);
}

void keyboardEnter(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(surface);
    Q_UNUSED(keys);

    static_cast<MockKeyboard *>(keyboard)->m_enteredSurface = surface;
}

void keyboardLeave(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface)
{
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(surface);

    static_cast<MockKeyboard *>(keyboard)->m_enteredSurface = nullptr;
}

void keyboardKey(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(key);
    Q_UNUSED(state);
    auto kb = static_cast<MockKeyboard *>(keyboard);
    kb->m_lastKeyCode = key;
    kb->m_lastKeyState = state;
}

void keyboardModifiers(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    auto kb = static_cast<MockKeyboard *>(keyboard);
    kb->m_group = group;
}

static const struct wl_keyboard_listener keyboardListener = {
    keyboardKeymap,
    keyboardEnter,
    keyboardLeave,
    keyboardKey,
    keyboardModifiers
};

MockKeyboard::MockKeyboard(wl_seat *seat)
    : m_keyboard(wl_seat_get_keyboard(seat))
{
    wl_keyboard_add_listener(m_keyboard, &keyboardListener, this);
}

MockKeyboard::~MockKeyboard()
{
    wl_keyboard_destroy(m_keyboard);
}
