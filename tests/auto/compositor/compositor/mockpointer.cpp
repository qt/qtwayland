// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mockpointer.h"

QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

static void pointerEnter(void *pointer, struct wl_pointer *wlPointer, uint serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);
    Q_UNUSED(x);
    Q_UNUSED(y);

    static_cast<MockPointer *>(pointer)->m_enteredSurface = surface;
}

static void pointerLeave(void *pointer, struct wl_pointer *wlPointer, uint32_t serial, struct wl_surface *surface)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);

    Q_ASSERT(surface);

    static_cast<MockPointer *>(pointer)->m_enteredSurface = nullptr;
}

static void pointerMotion(void *pointer, struct wl_pointer *wlPointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(time);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

static void pointerButton(void *pointer, struct wl_pointer *wlPointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(button);
    Q_UNUSED(state);
}

static void pointerAxis(void *pointer, struct wl_pointer *wlPointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(time);
    Q_UNUSED(axis);
    Q_UNUSED(value);
}

static const struct wl_pointer_listener pointerListener = {
    pointerEnter,
    pointerLeave,
    pointerMotion,
    pointerButton,
    pointerAxis,
};

MockPointer::MockPointer(wl_seat *seat)
    : m_pointer(wl_seat_get_pointer(seat))
{
    wl_pointer_add_listener(m_pointer, &pointerListener, this);
}

MockPointer::~MockPointer()
{
    wl_pointer_destroy(m_pointer);
}
