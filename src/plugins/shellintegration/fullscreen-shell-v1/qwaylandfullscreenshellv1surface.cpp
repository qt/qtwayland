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

#include <QtWaylandClient/private/qwaylandscreen_p.h>

#include "qwaylandfullscreenshellv1surface.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandFullScreenShellV1Surface::QWaylandFullScreenShellV1Surface(QtWayland::zwp_fullscreen_shell_v1 *shell, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , m_shell(shell)
    , m_window(window)
{
    auto *screen = m_window->waylandScreen();
    auto *output = screen ? screen->output() : nullptr;
    m_shell->present_surface(m_window->wlSurface(),
                             QtWayland::zwp_fullscreen_shell_v1::present_method_default,
                             output);
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
