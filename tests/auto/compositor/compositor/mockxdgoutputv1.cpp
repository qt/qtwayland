// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mockxdgoutputv1.h"

MockXdgOutputV1::MockXdgOutputV1(struct ::zxdg_output_v1 *object)
    : QtWayland::zxdg_output_v1(object)
{
}

MockXdgOutputV1::~MockXdgOutputV1()
{
    destroy();
}

void MockXdgOutputV1::zxdg_output_v1_logical_position(int32_t x, int32_t y)
{
    pending.logicalPosition = QPoint(x, y);
}

void MockXdgOutputV1::zxdg_output_v1_logical_size(int32_t width, int32_t height)
{
    pending.logicalSize = QSize(width, height);
}

void MockXdgOutputV1::zxdg_output_v1_done()
{
    // In version 3 we'll have to do this for wl_output.done as well
    name = pending.name;
    description = pending.description;
    logicalPosition = pending.logicalPosition;
    logicalSize = pending.logicalSize;
}

void MockXdgOutputV1::zxdg_output_v1_name(const QString &name)
{
    pending.name = name;
}

void MockXdgOutputV1::zxdg_output_v1_description(const QString &description)
{
    pending.description = description;
}
