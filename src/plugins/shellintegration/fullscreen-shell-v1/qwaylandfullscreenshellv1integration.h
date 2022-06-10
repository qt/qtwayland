// Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDFULLSCREENSHELLV1INTEGRATION_H
#define QWAYLANDFULLSCREENSHELLV1INTEGRATION_H

#include <wayland-client.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

#include "qwayland-fullscreen-shell-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLANDCLIENT_EXPORT QWaylandFullScreenShellV1Integration : public QWaylandShellIntegration
{
public:
    bool initialize(QWaylandDisplay *display) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

private:
    QScopedPointer<QtWayland::zwp_fullscreen_shell_v1> m_shell;
};

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDFULLSCREENSHELLV1INTEGRATION_H
