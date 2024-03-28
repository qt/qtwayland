// Copyright (C) 2016 LG Electronics Ltd., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mockseat.h"

MockSeat::MockSeat(wl_seat *seat)
    : m_seat(seat)
    , m_pointer(new MockPointer(seat))
    , m_keyboard(new MockKeyboard(seat))
{
}

MockSeat::~MockSeat()
{
    wl_seat_destroy(m_seat);
}
