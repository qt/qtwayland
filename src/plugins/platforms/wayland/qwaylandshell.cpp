/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandshell.h"

#include "qwaylandshellsurface.h"
#include "qwaylandwindow.h"

QWaylandShell::QWaylandShell(QWaylandDisplay *display, uint32_t id, uint32_t version)
    : m_display(display)
{
    Q_UNUSED(version)
    m_shell = static_cast<struct wl_shell *>(wl_display_bind(m_display->wl_display(), id, &wl_shell_interface));
}

QWaylandShellSurface *QWaylandShell::createShellSurface(QWaylandWindow *window)
{
    Q_ASSERT(window->wl_surface());
    struct wl_shell_surface *shell_surface = wl_shell_get_shell_surface(m_shell,window->wl_surface());
    return new QWaylandShellSurface(shell_surface,window);
}
