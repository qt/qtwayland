// Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandfullscreenshellv1integration.h"
#include "qwaylandfullscreenshellv1surface.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandFullScreenShellV1Integration::initialize(QWaylandDisplay *display)
{
    for (const QWaylandDisplay::RegistryGlobal &global : display->globals()) {
        if (global.interface == QLatin1String("zwp_fullscreen_shell_v1") && !m_shell) {
            m_shell.reset(new QtWayland::zwp_fullscreen_shell_v1(display->wl_registry(), global.id, global.version));
            break;
        }
    }

    if (!m_shell) {
        qCDebug(lcQpaWayland) << "Couldn't find global zwp_fullscreen_shell_v1 for fullscreen-shell";
        return false;
    }

    return true;
}

QWaylandShellSurface *QWaylandFullScreenShellV1Integration::createShellSurface(QWaylandWindow *window)
{
    return new QWaylandFullScreenShellV1Surface(m_shell.data(), window);
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
