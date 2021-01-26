/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qwaylandxdgshellv6integration_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellV6Integration::initialize(QWaylandDisplay *display)
{
    for (QWaylandDisplay::RegistryGlobal global : display->globals()) {
        if (global.interface == QLatin1String("zxdg_shell_v6")) {
            m_xdgShell.reset(new QWaylandXdgShellV6(display->wl_registry(), global.id, global.version));
            break;
        }
    }

    if (!m_xdgShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global zxdg_shell_v6 for xdg-shell unstable v6";
        return false;
    }

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandXdgShellV6Integration::createShellSurface(QWaylandWindow *window)
{
    return m_xdgShell->getXdgSurface(window);
}

void QWaylandXdgShellV6Integration::handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus)
{
    if (newFocus) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurfaceV6 *>(newFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowActivated(newFocus);
    }
    if (oldFocus && qobject_cast<QWaylandXdgSurfaceV6 *>(oldFocus->shellSurface())) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurfaceV6 *>(oldFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowDeactivated(oldFocus);
    }
}

}

QT_END_NAMESPACE
