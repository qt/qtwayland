/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "qwaylandqtshellintegration.h"

#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include "qwaylandqtsurface_p.h"

#include <mutex>

#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandQtShellIntegration::QWaylandQtShellIntegration()
{
}

bool QWaylandQtShellIntegration::initialize()
{
    if (m_qtShell)
        return true;
    wl_registry *registry;
    uint32_t id;
    uint32_t version;
    bool found = findGlobal(QLatin1String("zqt_shell_v1"), &registry, &id, &version);
    if (!found) {
        qCDebug(lcQpaWayland) << "Couldn't find global zqt_shell_v1 for qt-shell";
        return false;
    }
    m_qtShell.reset(new QtWayland::zqt_shell_v1(registry, id, version));
    return true;
}

QWaylandShellSurface *QWaylandQtShellIntegration::createShellSurface(QWaylandWindow *window)
{
    if (!m_qtShell)
        return nullptr;

    auto *surface = m_qtShell->surface_create(wlSurfaceForWindow(window));
    return new QWaylandQtSurface(surface, window);
}

}

QT_END_NAMESPACE
