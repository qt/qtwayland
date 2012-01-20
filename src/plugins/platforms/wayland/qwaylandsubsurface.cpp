/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylandsubsurface.h"

#include "qwaylandwindow.h"

#include "wayland-sub-surface-extension-client-protocol.h"

QWaylandSubSurfaceExtension::QWaylandSubSurfaceExtension(QWaylandDisplay *display, uint32_t id)
{
    m_sub_surface_extension = static_cast<struct wl_sub_surface_extension *>(
                wl_display_bind(display->wl_display(),id, &wl_sub_surface_extension_interface));
}

QWaylandSubSurface *QWaylandSubSurfaceExtension::getSubSurfaceAwareWindow(QWaylandWindow *window)
{
    struct wl_surface *surface = window->wl_surface();
    Q_ASSERT(surface);
    struct wl_sub_surface *sub_surface =
            wl_sub_surface_extension_get_sub_surface_aware_surface(m_sub_surface_extension,surface);

    return new QWaylandSubSurface(window,sub_surface);

}

QWaylandSubSurface::QWaylandSubSurface(QWaylandWindow *window, struct wl_sub_surface *sub_surface)
    : m_window(window)
    , m_sub_surface(sub_surface)
{
}

void QWaylandSubSurface::setParent(const QWaylandWindow *parent)
{
    QWaylandSubSurface *parentSurface = parent? parent->subSurfaceWindow():0;
    if (parentSurface) {
        int x = m_window->geometry().x();
        int y = m_window->geometry().y();
        wl_sub_surface_attach_sub_surface(parentSurface->m_sub_surface,m_sub_surface,x,y);
    }
}
