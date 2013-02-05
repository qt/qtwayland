/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandsubsurface.h"

#include "qwaylandwindow.h"

#include "wayland-sub-surface-extension-client-protocol.h"

#include <QtCore/QDebug>

QT_USE_NAMESPACE

QWaylandSubSurfaceExtension::QWaylandSubSurfaceExtension(QWaylandDisplay *display, uint32_t id)
{
    m_sub_surface_extension = static_cast<struct wl_sub_surface_extension *>(
                wl_registry_bind(display->wl_registry(), id, &wl_sub_surface_extension_interface, 1));
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
        int x = m_window->geometry().x() + parent->frameMargins().left();
        int y = m_window->geometry().y() + parent->frameMargins().top();
        wl_sub_surface_attach_sub_surface(parentSurface->m_sub_surface,m_sub_surface,x,y);
    }
}

static void setPositionToParent(QWaylandWindow *parentWaylandWindow)
{
    QObjectList children = parentWaylandWindow->window()->children();
    for (int i = 0; i < children.size(); i++) {
        QWindow *childWindow = qobject_cast<QWindow *>(children.at(i));
        if (!childWindow)
            continue;

        if (childWindow->handle()) {
            QWaylandWindow *waylandWindow = static_cast<QWaylandWindow *>(childWindow->handle());
            waylandWindow->subSurfaceWindow()->setParent(parentWaylandWindow);
            setPositionToParent(waylandWindow);
        }
    }
}

void QWaylandSubSurface::adjustPositionOfChildren()
{
    QWindow *window = m_window->window();
    if (window->parent()) {
        qDebug() << "QWaylandSubSurface::adjustPositionOfChildren not called for toplevel window";
    }
    setPositionToParent(m_window);
}
