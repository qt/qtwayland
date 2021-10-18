/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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

#include "qwaylandxdgshellintegration_p.h"
#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellIntegration::initialize()
{
    if (m_xdgShell)
        return true;
    wl_registry *registry;
    uint32_t id;
    uint32_t version;
    bool found = findGlobal(QLatin1String("xdg_wm_base"), &registry, &id, &version);
    if (found)
        m_xdgShell.reset(new QWaylandXdgShell(m_display, id, version));
    if (!m_xdgShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global xdg_wm_base for xdg-shell stable";
        return false;
    }
    return true;
}

QWaylandShellSurface *QWaylandXdgShellIntegration::createShellSurface(QWaylandWindow *window)
{
    return m_xdgShell->getXdgSurface(window);
}

void *QWaylandXdgShellIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    if (auto waylandWindow = static_cast<QWaylandWindow *>(window->handle())) {
        if (auto xdgSurface = qobject_cast<QWaylandXdgSurface *>(waylandWindow->shellSurface())) {
            return xdgSurface->nativeResource(resource);
        }
    }
    return nullptr;
}

}

QT_END_NAMESPACE
