// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exampleshellintegration.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandSeat>
#include "exampleshell.h"


ExampleShellIntegration::ExampleShellIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<ExampleShellSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &ExampleShellSurface::destroyed, this, &ExampleShellIntegration::handleExampleShellSurfaceDestroyed);
}

ExampleShellIntegration::~ExampleShellIntegration()
{
    m_item->setSurface(nullptr);
}

void ExampleShellIntegration::handleExampleShellSurfaceDestroyed()
{
    m_shellSurface = nullptr;
}
