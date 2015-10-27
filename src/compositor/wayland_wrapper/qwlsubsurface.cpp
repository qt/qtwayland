/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>

#include "qwlsubsurface_p.h"

#include "qwlcompositor_p.h"
#include "qwaylandsurface.h"
#include "qwaylandsurfaceview.h"

#include "qwaylandsurfaceitem.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

SubSurface::SubSurface(Surface *surface, Surface *parent, wl_client *client, uint32_t id, int version)
    : QtWaylandServer::wl_subsurface(client, id, version)
    , m_surface(surface)
    , m_parent(parent)
    , m_synchronized(true)
{
    m_surface->setSubSurface(this);
    parent->addSubSurface(this);
    QWaylandSurface *p = parent->waylandSurface();

    foreach (QWaylandSurfaceView *v, p->views())
        createSubView(v);
    connect(p, &QWaylandSurface::viewAdded, this, &SubSurface::createSubView);
}

SubSurface::~SubSurface()
{
    qDeleteAll(m_views);

    m_surface->setSubSurface(Q_NULLPTR);
    m_parent->removeSubSurface(this);
}

const SurfaceRole *SubSurface::role()
{
    static const SurfaceRole role = { "subsurface" };
    return &role;
}

void SubSurface::parentCommit()
{
    foreach (QWaylandSurfaceView *view, m_views) {
        view->setPos(m_position);
    }
}

void SubSurface::configure(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)
}

void SubSurface::createSubView(QWaylandSurfaceView *view)
{
    QWaylandSurfaceView *v = m_surface->compositor()->waylandCompositor()->createView(m_surface->waylandSurface());
    v->setParentView(view);
    v->setPos(m_position);
    m_views << v;
}

void SubSurface::subsurface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void SubSurface::subsurface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void SubSurface::subsurface_set_position(Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource)

    m_position = QPoint(x, y);
}

void SubSurface::subsurface_place_above(Resource *resource, ::wl_resource *sibling)
{
    Q_UNUSED(resource)
    Q_UNUSED(sibling)
    qWarning("wl_subsurface.place_above not implemented");
}

void SubSurface::subsurface_place_below(Resource *resource, ::wl_resource *sibling)
{
    Q_UNUSED(resource)
    Q_UNUSED(sibling)
    qWarning("wl_subsurface.place_below not implemented");
}

void SubSurface::subsurface_set_sync(Resource *resource)
{
    Q_UNUSED(resource)
    qWarning("wl_subsurface.set_sync not implemented");
}

void SubSurface::subsurface_set_desync(Resource *resource)
{
    Q_UNUSED(resource)
    qWarning("wl_subsurface.set_desync not implemented");
}

}

QT_END_NAMESPACE
