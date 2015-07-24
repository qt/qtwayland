/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <wayland-server.h>

#include "qwaylandsurfaceinterface.h"
#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"

class QWaylandSurfaceInterface::Private
{
public:
    QWaylandSurface *surface;
};

QWaylandSurfaceInterface::QWaylandSurfaceInterface(QWaylandSurface *surface)
                   : d(new Private)
{
    d->surface = surface;
    surface->addInterface(this);
}

QWaylandSurfaceInterface::~QWaylandSurfaceInterface()
{
    d->surface->removeInterface(this);
    delete d;
}

QWaylandSurface *QWaylandSurfaceInterface::surface() const
{
    return d->surface;
}

void QWaylandSurfaceInterface::setSurfaceType(QWaylandSurface::WindowType type)
{
    surface()->d_func()->setType(type);
}

void QWaylandSurfaceInterface::setSurfaceClassName(const QString &name)
{
    surface()->d_func()->setClassName(name);
}

void QWaylandSurfaceInterface::setSurfaceTitle(const QString &title)
{
    surface()->d_func()->setTitle(title);
}



class QWaylandSurfaceOp::Private
{
public:
    int type;
};

QWaylandSurfaceOp::QWaylandSurfaceOp(int t)
                 : d(new Private)
{
    d->type = t;
}

QWaylandSurfaceOp::~QWaylandSurfaceOp()
{
    delete d;
}

int QWaylandSurfaceOp::type() const
{
    return d->type;
}



QWaylandSurfaceSetVisibilityOp::QWaylandSurfaceSetVisibilityOp(QWindow::Visibility visibility)
                              : QWaylandSurfaceOp(QWaylandSurfaceOp::SetVisibility)
                              , m_visibility(visibility)
{
}

QWindow::Visibility QWaylandSurfaceSetVisibilityOp::visibility() const
{
    return m_visibility;
}

QWaylandSurfaceResizeOp::QWaylandSurfaceResizeOp(const QSize &size)
                       : QWaylandSurfaceOp(QWaylandSurfaceOp::Resize)
                       , m_size(size)
{
}

QSize QWaylandSurfaceResizeOp::size() const
{
    return m_size;
}

QWaylandSurfacePingOp::QWaylandSurfacePingOp(uint32_t serial)
                     : QWaylandSurfaceOp(QWaylandSurfaceOp::Ping)
                     , m_serial(serial)
{
}

uint32_t QWaylandSurfacePingOp::serial() const
{
    return m_serial;
}
