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

#include "mockcompositor.h"
#include "mocksurface.h"

#include <qwayland-server-xdg-shell-unstable-v6.h>

namespace Impl {

class XdgToplevelV6 : public QtWaylandServer::zxdg_toplevel_v6
{
public:
    XdgToplevelV6(wl_client *client, uint32_t id, int version)
        : QtWaylandServer::zxdg_toplevel_v6(client, id, version)
    {}
    void zxdg_toplevel_v6_destroy_resource(Resource *resource) override { delete this; }
};

class XdgSurfaceV6 : public QtWaylandServer::zxdg_surface_v6
{
public:
    XdgSurfaceV6(wl_client *client, uint32_t id, int version, Surface *surface);
    void zxdg_surface_v6_destroy_resource(Resource *resource) override { delete this; }
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    Surface *m_surface = nullptr;
};

XdgSurfaceV6::XdgSurfaceV6(wl_client *client, uint32_t id, int version, Surface *surface)
    : QtWaylandServer::zxdg_surface_v6(client, id, version)
    , m_surface(surface)
{
}

void XdgSurfaceV6::zxdg_surface_v6_get_toplevel(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t id)
{
    int version = wl_resource_get_version(resource->handle);
    new XdgToplevelV6(resource->client(), id, version);
    m_surface->map();
}


void shell_destroy(struct wl_client *client,
             struct wl_resource *resource)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
}

void create_positioner(struct wl_client *client,
                       struct wl_resource *resource,
                       uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(id);
}

void get_xdg_surface(struct wl_client *client,
                     struct wl_resource *compositorResource,
                     uint32_t id,
                     struct wl_resource *surfaceResource)
{
    int version = wl_resource_get_version(compositorResource);
    new XdgSurfaceV6(client, id, version, Surface::fromResource(surfaceResource));
}

void pong(struct wl_client *client,
          struct wl_resource *resource,
          uint32_t serial)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(serial);
}

void Compositor::bindXdgShellV6(wl_client *client, void *compositorData, uint32_t version, uint32_t id)
{
    static const struct zxdg_shell_v6_interface shellInterface = {
        shell_destroy,
        create_positioner,
        get_xdg_surface,
        pong
    };

    wl_resource *resource = wl_resource_create(client, &zxdg_shell_v6_interface, static_cast<int>(version), id);
    wl_resource_set_implementation(resource, &shellInterface, compositorData, nullptr);
}

}
