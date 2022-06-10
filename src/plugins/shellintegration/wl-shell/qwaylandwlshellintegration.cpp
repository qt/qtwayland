// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
                            << "\"xdg-shell\" if supported by the compositor"
                            << "by setting the environment variable QT_WAYLAND_SHELL_INTEGRATION";

    return true;
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
