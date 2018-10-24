/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef MOCKCOMPOSITOR_XDGSHELL_H
#define MOCKCOMPOSITOR_XDGSHELL_H

#include "coreprotocol.h"
#include <qwayland-server-xdg-shell.h>

namespace MockCompositor {

class XdgSurface;
class XdgToplevel;
class XdgPopup;
using XdgPositioner = QtWaylandServer::xdg_positioner;

class XdgWmBase : public Global, public QtWaylandServer::xdg_wm_base
{
    Q_OBJECT
public:
    explicit XdgWmBase(CoreCompositor *compositor, int version = 1);
    using QtWaylandServer::xdg_wm_base::send_ping;
    void send_ping(uint32_t) = delete; // It's a global, use resource specific instead
    bool isClean() override { return m_xdgSurfaces.empty(); }
    QString dirtyMessage() override { return m_xdgSurfaces.empty() ? "clean" : "remaining xdg surfaces"; }
    QVector<XdgSurface *> m_xdgSurfaces;
    XdgToplevel *toplevel(int i = 0);
    XdgPopup *popup(int i = 0);
    CoreCompositor *m_compositor = nullptr;

signals:
    void pong(uint serial);
    void xdgSurfaceCreated(XdgSurface *xdgSurface);
    void toplevelCreated(XdgToplevel *toplevel);

protected:
    void xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;
    void xdg_wm_base_pong(Resource *resource, uint32_t serial) override;
    void xdg_wm_base_create_positioner(Resource *resource, uint32_t id) override
    {
        new XdgPositioner(resource->client(), id, resource->version());
    }
};

class XdgSurface : public QObject, public QtWaylandServer::xdg_surface
{
    Q_OBJECT
public:
    explicit XdgSurface(XdgWmBase *xdgWmBase, Surface *surface, wl_client *client, int id, int version);
    void send_configure(uint serial) = delete; // Use the one below instead, as it tracks state
    void sendConfigure(uint serial);
    uint sendConfigure();
    XdgToplevel *m_toplevel = nullptr;
    XdgPopup *m_popup = nullptr;
    XdgWmBase *m_xdgWmBase = nullptr;
    Surface *m_surface = nullptr;
    bool m_configureSent = false;
    QVector<uint> m_pendingConfigureSerials;
    uint m_ackedConfigureSerial = 0;
    uint m_committedConfigureSerial = 0;

public slots:
    void verifyConfigured() { QVERIFY(m_configureSent); }

signals:
    void configureCommitted(uint);
    void toplevelCreated(XdgToplevel *toplevel);

protected:
    void xdg_surface_get_toplevel(Resource *resource, uint32_t id) override;
    void xdg_surface_get_popup(Resource *resource, uint32_t id, ::wl_resource *parent, ::wl_resource *positioner) override;
    void xdg_surface_destroy_resource(Resource *resource) override;
    void xdg_surface_destroy(Resource *resource) override { wl_resource_destroy(resource->handle); }
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) override;
};

class XdgToplevel : public QtWaylandServer::xdg_toplevel
{
public:
    explicit XdgToplevel(XdgSurface *xdgSurface, int id, int version = 1);
    void sendConfigure(const QSize &size = {0, 0}, const QVector<uint> &states = {});
    uint sendCompleteConfigure(const QSize &size = {0, 0}, const QVector<uint> &states = {});
    Surface *surface() { return m_xdgSurface->m_surface; }
    XdgSurface *m_xdgSurface = nullptr;
};

class XdgPopup : public QtWaylandServer::xdg_popup
{
public:
    explicit XdgPopup(XdgSurface *xdgSurface, int id, int version = 1);
    void sendConfigure(const QRect &geometry);
    Surface *surface() { return m_xdgSurface->m_surface; }
    XdgSurface *m_xdgSurface = nullptr;
    bool m_grabbed = false;
    uint m_grabSerial = 0;
protected:
    void xdg_popup_grab(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
};

} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_XDGSHELL_H
