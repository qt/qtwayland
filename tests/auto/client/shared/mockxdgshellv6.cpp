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
#include "mockcompositor.h"

namespace Impl {

void Compositor::sendXdgToplevelV6Configure(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    XdgToplevelV6 *toplevel = resolveToplevel(parameters.at(0));
    Q_ASSERT(toplevel && toplevel->resource());
    QSize size = parameters.at(1).toSize();
    Q_ASSERT(size.isValid());
    QByteArray states;
    toplevel->send_configure(size.width(), size.height(), states);
    toplevel->xdgSurface()->send_configure(compositor->nextSerial());
}

XdgSurfaceV6::XdgSurfaceV6(XdgShellV6 *shell, Surface *surface, wl_client *client, uint32_t id)
    : QtWaylandServer::zxdg_surface_v6(client, id, 1)
    , m_surface(surface)
    , m_shell(shell)
{
}

void XdgSurfaceV6::zxdg_surface_v6_get_toplevel(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t id)
{
    int version = wl_resource_get_version(resource->handle);
    m_toplevel = new XdgToplevelV6(this, resource->client(), id, version);
}

void XdgSurfaceV6::zxdg_surface_v6_destroy(QtWaylandServer::zxdg_surface_v6::Resource *resource)
{
    Q_ASSERT(!m_toplevel);
    wl_resource_destroy(resource->handle);
}

XdgToplevelV6::XdgToplevelV6(XdgSurfaceV6 *xdgSurface, wl_client *client, uint32_t id, int version)
    : QtWaylandServer::zxdg_toplevel_v6(client, id, version)
    , m_xdgSurface(xdgSurface)
    , m_mockToplevel(new MockXdgToplevelV6(this))
{
    auto *surface = m_xdgSurface->surface();
    surface->m_xdgToplevelV6 = this;
    m_xdgSurface->shell()->addToplevel(this);
    surface->map();
}

XdgToplevelV6::~XdgToplevelV6()
{
    m_xdgSurface->shell()->removeToplevel(this);
    m_xdgSurface->surface()->m_xdgToplevelV6 = nullptr;
    m_mockToplevel->m_toplevel = nullptr;
}

void XdgToplevelV6::zxdg_toplevel_v6_destroy(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    m_xdgSurface->m_toplevel = nullptr;
    wl_resource_destroy(resource->handle);
}

void Impl::XdgShellV6::zxdg_shell_v6_get_xdg_surface(QtWaylandServer::zxdg_shell_v6::Resource *resource, uint32_t id, wl_resource *surface)
{
    new XdgSurfaceV6(this, Surface::fromResource(surface), resource->client(), id);
}

} // namespace Impl
