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

#include "qwaylandwlshellintegration_p.h"
#include "qwaylandwlshellsurface_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandWlShellIntegration::initialize(QWaylandDisplay *display)
{
    const auto globals = display->globals();
    for (QWaylandDisplay::RegistryGlobal global : globals) {
        if (global.interface == QLatin1String("wl_shell")) {
            m_wlShell = new QtWayland::wl_shell(display->wl_registry(), global.id, 1);
            break;
        }
    }

    if (!m_wlShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global wl_shell";
        return false;
    }

    qCWarning(lcQpaWayland) << "\"wl-shell\" is a deprecated shell extension, prefer using"
                            << "\"xdg-shell-v6\" or \"xdg-shell\" if supported by the compositor"
                            << "by setting the environment variable QT_WAYLAND_SHELL_INTEGRATION";

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandWlShellIntegration::createShellSurface(QWaylandWindow *window)
{
    return new QWaylandWlShellSurface(m_wlShell->get_shell_surface(window->wlSurface()), window);
}

void *QWaylandWlShellIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    QByteArray lowerCaseResource = resource.toLower();
    if (lowerCaseResource == "wl_shell_surface") {
        if (auto waylandWindow = static_cast<QWaylandWindow *>(window->handle())) {
            if (auto shellSurface = qobject_cast<QWaylandWlShellSurface *>(waylandWindow->shellSurface())) {
                return shellSurface->object();
            }
        }
    }
    return nullptr;
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
