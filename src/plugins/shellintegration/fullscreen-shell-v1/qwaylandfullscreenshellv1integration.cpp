/****************************************************************************
**
** Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandFullScreenShellV1Integration::createShellSurface(QWaylandWindow *window)
{
    return new QWaylandFullScreenShellV1Surface(m_shell.data(), window);
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
