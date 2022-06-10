// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EXAMPLESHELLINTEGRATION_H
#define EXAMPLESHELLINTEGRATION_H

#include "exampleshell.h"
#include <QtWaylandCompositor/QWaylandQuickShellIntegration>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>

class ExampleShellIntegration : public  QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    ExampleShellIntegration(QWaylandQuickShellSurfaceItem *item);
    ~ExampleShellIntegration() override;

private slots:
    void handleExampleShellSurfaceDestroyed();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    ExampleShellSurface *m_shellSurface = nullptr;
};

#endif // EXAMPLESHELLINTEGRATION_H
