/****************************************************************************
**
** Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: http://www.qt-project.org/legal
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

#ifndef QWAYLANDFULLSCREENSHELLV1SURFACE_H
#define QWAYLANDFULLSCREENSHELLV1SURFACE_H

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include "qwayland-fullscreen-shell-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandFullScreenShellV1Surface : public QWaylandShellSurface
{
public:
    QWaylandFullScreenShellV1Surface(QtWayland::zwp_fullscreen_shell_v1 *shell, QWaylandWindow *window);

private:
    QtWayland::zwp_fullscreen_shell_v1 *m_shell = nullptr;
    QWaylandWindow *m_window = nullptr;
};

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDFULLSCREENSHELLV1SURFACE_H
