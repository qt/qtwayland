/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include "qwlsubsurface_p.h"

#include "qwlcompositor_p.h"
#include "qwaylandsurface.h"
#include "qwaylandview.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

SubSurfaceExtensionGlobal::SubSurfaceExtensionGlobal(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate(compositor)
    , qt_sub_surface_extension(compositor->waylandDisplay(), 1)
    , m_compositor(compositor)
{
}

void SubSurfaceExtensionGlobal::sub_surface_extension_get_sub_surface_aware_surface(Resource *resource,
                                                                                    uint32_t id,
                                                                                    struct ::wl_resource *surface)
{
    QWaylandSurface *waylandsurface = QWaylandSurface::fromResource(surface);
    new SubSurface(resource->client(),id,waylandsurface);
}

SubSurface::SubSurface(wl_client *client, uint32_t id, QWaylandSurface *surface)
    : QWaylandExtensionTemplate(surface)
    , qt_sub_surface(client, id, 1)
    , m_surface(surface)
    , m_parent(0)
{
}

SubSurface::~SubSurface()
{
    if (m_parent) {
        m_parent->removeSubSurface(this);
    }
    QLinkedList<QWaylandSurface *>::iterator it;
    for (it = m_sub_surfaces.begin(); it != m_sub_surfaces.end(); ++it) {
        (*it)->handle()->subSurface()->parentDestroyed();
    }
}

void SubSurface::setSubSurface(SubSurface *subSurface, int x, int y)
{
    if (!m_sub_surfaces.contains(subSurface->m_surface)) {
        m_sub_surfaces.append(subSurface->m_surface);
        subSurface->setParent(this);
    }
    foreach (QWaylandView *view, subSurface->m_surface->views())
        view->setRequestedPosition(QPointF(x,y));
}

void SubSurface::removeSubSurface(SubSurface *subSurfaces)
{
    Q_ASSERT(m_sub_surfaces.contains(subSurfaces->m_surface));
    m_sub_surfaces.removeOne(subSurfaces->m_surface);
}

SubSurface *SubSurface::parent() const
{
    return m_parent;
}

void SubSurface::setParent(SubSurface *parent)
{
    if (m_parent == parent)
        return;

    SubSurface *oldParent = 0;
    SubSurface *newParent = 0;

    if (m_parent) {
        oldParent = m_parent;
        m_parent->removeSubSurface(this);
    }
    if (parent) {
        newParent = parent;
    }
    m_parent = parent;

    parentChanged(newParent,oldParent);
}

QLinkedList<QWaylandSurface *> SubSurface::subSurfaces() const
{
    return m_sub_surfaces;
}

void SubSurface::parentDestroyed()
{
    m_parent = 0;
}

void SubSurface::sub_surface_attach_sub_surface(Resource *resource, struct ::wl_resource *sub_surface, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    SubSurface *child_sub_surface = static_cast<SubSurface *>(Resource::fromResource(sub_surface)->sub_surface_object);
    setSubSurface(child_sub_surface,x,y);
}

void SubSurface::sub_surface_move_sub_surface(Resource *resource, struct ::wl_resource *sub_surface, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(sub_surface);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void SubSurface::sub_surface_raise(Resource *resource, struct ::wl_resource *sub_surface)
{
    Q_UNUSED(resource);
    Q_UNUSED(sub_surface);
}

void SubSurface::sub_surface_lower(Resource *resource, struct ::wl_resource *sub_surface)
{
    Q_UNUSED(resource);
    Q_UNUSED(sub_surface);
}

}

QT_END_NAMESPACE
