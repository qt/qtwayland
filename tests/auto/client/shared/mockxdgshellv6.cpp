/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockxdgshellv6.h"
#include "mocksurface.h"

namespace Impl {

class XdgSurfaceV6;

class XdgToplevelV6 : public QtWaylandServer::zxdg_toplevel_v6
{
public:
    XdgToplevelV6(XdgSurfaceV6 *xdgSurface, wl_client *client, uint32_t id, int version)
        : QtWaylandServer::zxdg_toplevel_v6(client, id, version)
        , m_xdgSurface(xdgSurface)
    {}
    void zxdg_toplevel_v6_destroy_resource(Resource *) override { delete this; }
    void zxdg_toplevel_v6_destroy(Resource *resource) override;
    XdgSurfaceV6 *m_xdgSurface = nullptr;
};

class XdgSurfaceV6 : public QtWaylandServer::zxdg_surface_v6
{
public:
    XdgSurfaceV6(wl_client *client, uint32_t id, Surface *surface);
    void zxdg_surface_v6_destroy_resource(Resource *) override { delete this; }
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    void zxdg_surface_v6_destroy(Resource *resource) override
    {
        Q_ASSERT(!m_toplevel);
        wl_resource_destroy(resource->handle);
    }
    Surface *m_surface = nullptr;
    XdgToplevelV6 *m_toplevel = nullptr;
};

void XdgToplevelV6::zxdg_toplevel_v6_destroy(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    m_xdgSurface->m_toplevel = nullptr;
    wl_resource_destroy(resource->handle);
}

XdgSurfaceV6::XdgSurfaceV6(wl_client *client, uint32_t id, Surface *surface)
    : QtWaylandServer::zxdg_surface_v6(client, id, 1)
    , m_surface(surface)
{
}

void XdgSurfaceV6::zxdg_surface_v6_get_toplevel(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t id)
{
    int version = wl_resource_get_version(resource->handle);
    m_toplevel = new XdgToplevelV6(this, resource->client(), id, version);
    m_surface->map();
}

void Impl::XdgShellV6::zxdg_shell_v6_get_xdg_surface(QtWaylandServer::zxdg_shell_v6::Resource *resource, uint32_t id, wl_resource *surface)
{
    new XdgSurfaceV6(resource->client(), id, Surface::fromResource(surface));
}

} // namespace Impl
