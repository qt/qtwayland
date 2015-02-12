/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandsubsurface_p.h"

#include "qwaylandwindow_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandSubSurface::QWaylandSubSurface(QWaylandWindow *window, struct ::qt_sub_surface *sub_surface)
    : QtWayland::qt_sub_surface(sub_surface)
    , m_window(window)
{
}

void QWaylandSubSurface::setParent(const QWaylandWindow *parent)
{
    QWaylandSubSurface *parentSurface = parent ? parent->subSurfaceWindow() : 0;
    if (parentSurface) {
        int x = m_window->geometry().x() + parent->frameMargins().left();
        int y = m_window->geometry().y() + parent->frameMargins().top();
        parentSurface->attach_sub_surface(object(), x, y);
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

}

QT_END_NAMESPACE
