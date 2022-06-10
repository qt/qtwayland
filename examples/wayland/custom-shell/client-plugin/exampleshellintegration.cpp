// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exampleshellintegration.h"
#include "examplesurface.h"

//! [constructor]
ExampleShellIntegration::ExampleShellIntegration()
    : QWaylandShellIntegrationTemplate(/* Supported protocol version */ 1)
{
}
//! [constructor]

//! [createShellSurface]
QWaylandShellSurface *ExampleShellIntegration::createShellSurface(QWaylandWindow *window)
{
    if (!isActive())
        return nullptr;
    auto *surface = surface_create(wlSurfaceForWindow(window));
    return new ExampleShellSurface(surface, window);
}
//! [createShellSurface]
