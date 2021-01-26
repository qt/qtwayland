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

#include "qwaylandxdgshellintegration_p.h"
#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellIntegration::initialize(QWaylandDisplay *display)
{
    for (QWaylandDisplay::RegistryGlobal global : display->globals()) {
        if (global.interface == QLatin1String("xdg_wm_base")) {
            m_xdgShell.reset(new QWaylandXdgShell(display, global.id, global.version));
            break;
        }
    }

    if (!m_xdgShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global xdg_wm_base for xdg-shell stable";
        return false;
    }

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandXdgShellIntegration::createShellSurface(QWaylandWindow *window)
{
    return m_xdgShell->getXdgSurface(window);
}

void QWaylandXdgShellIntegration::handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus)
{
    if (newFocus) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurface *>(newFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowActivated(newFocus);
    }
    if (oldFocus && qobject_cast<QWaylandXdgSurface *>(oldFocus->shellSurface())) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurface *>(oldFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowDeactivated(oldFocus);
    }
}

}

QT_END_NAMESPACE
