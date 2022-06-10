// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "examplesurface.h"
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformwindow.h>

ExampleShellSurface::ExampleShellSurface(struct ::qt_example_shell_surface *shell_surface, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , QtWayland::qt_example_shell_surface(shell_surface)
{
}

ExampleShellSurface::~ExampleShellSurface()
{
}

bool ExampleShellSurface::wantsDecorations() const
{
    return true;
}

//! [setTitle]
void ExampleShellSurface::setTitle(const QString &windowTitle)
{
    set_window_title(windowTitle);
}
//! [setTitle]

void ExampleShellSurface::requestWindowStates(Qt::WindowStates states)
{
    set_minimized(states & Qt::WindowMinimized);
}

//! [applyConfigure]
void ExampleShellSurface::applyConfigure()
{
    if (m_stateChanged)
        QWindowSystemInterface::handleWindowStateChanged(platformWindow()->window(), m_pendingStates);
    m_stateChanged = false;
}
//! [applyConfigure]

void ExampleShellSurface::example_shell_surface_minimize(uint32_t minimized)
{
    m_pendingStates = minimized ? Qt::WindowMinimized : Qt::WindowNoState;
    m_stateChanged = true;
    applyConfigureWhenPossible();
}
