/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
