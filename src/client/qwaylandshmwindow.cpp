// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandshmwindow_p.h"

#include "qwaylandbuffer_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandShmWindow::QWaylandShmWindow(QWindow *window, QWaylandDisplay *display)
    : QWaylandWindow(window, display)
{
}

QWaylandShmWindow::~QWaylandShmWindow()
{
}

QWaylandWindow::WindowType QWaylandShmWindow::windowType() const
{
    return QWaylandWindow::Shm;
}

}

QT_END_NAMESPACE
