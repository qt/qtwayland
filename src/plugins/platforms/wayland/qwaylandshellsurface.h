/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

#include <inttypes.h>

class QWaylandWindow;

class QWaylandShellSurface
{
public:
    QWaylandShellSurface(struct wl_shell_surface *shell_surface, QWaylandWindow *window);

    struct wl_shell_surface *handle() const { return m_shell_surface; }
private:
    struct wl_shell_surface *m_shell_surface;
    QWaylandWindow *m_window;

    static void configure(void *data,
              struct wl_shell_surface *wl_shell_surface,
              uint32_t time,
              uint32_t edges,
              int32_t width,
              int32_t height);
    static const struct wl_shell_surface_listener m_shell_surface_listener;
};

#endif // QWAYLANDSHELLSURFACE_H
