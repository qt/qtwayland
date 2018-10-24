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

#include "xdgshell.h"

namespace MockCompositor {

XdgWmBase::XdgWmBase(CoreCompositor *compositor, int version)
    : QtWaylandServer::xdg_wm_base(compositor->m_display, version)
    , m_compositor(compositor)
{
}

XdgToplevel *XdgWmBase::toplevel(int i)
{
    int j = 0;
    for (auto *xdgSurface : qAsConst(m_xdgSurfaces)) {
        if (auto *toplevel = xdgSurface->m_toplevel) {
            if (j == i)
                return toplevel;
            ++j;
        }
    }
    return nullptr;
}

XdgPopup *XdgWmBase::popup(int i)
{
    int j = 0;
    for (auto *xdgSurface : qAsConst(m_xdgSurfaces)) {
        if (auto *popup = xdgSurface->m_popup) {
            if (j == i)
                return popup;
            ++j;
        }
    }
    return nullptr;
}

void XdgWmBase::xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surface)
{
    auto *s = fromResource<Surface>(surface);
    auto *xdgSurface = new XdgSurface(this, s, resource->client(), id, resource->version());
    m_xdgSurfaces << xdgSurface;
    emit xdgSurfaceCreated(xdgSurface);
}

void XdgWmBase::xdg_wm_base_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    emit pong(serial);
}

XdgSurface::XdgSurface(XdgWmBase *xdgWmBase, Surface *surface, wl_client *client, int id, int version)
    : QtWaylandServer::xdg_surface(client, id, version)
    , m_xdgWmBase(xdgWmBase)
    , m_surface(surface)
{
    QVERIFY(!surface->m_pending.buffer);
    QVERIFY(!surface->m_committed.buffer);
    connect(this, &XdgSurface::toplevelCreated, xdgWmBase, &XdgWmBase::toplevelCreated);
    connect(surface, &Surface::attach, this, &XdgSurface::verifyConfigured);
    connect(surface, &Surface::commit, this, [this] {
        if (m_ackedConfigureSerial != m_committedConfigureSerial) {
            m_committedConfigureSerial = m_ackedConfigureSerial;
            emit configureCommitted(m_committedConfigureSerial);
        }
    });
}

void XdgSurface::sendConfigure(uint serial)
{
    Q_ASSERT(serial);
    m_pendingConfigureSerials.append(serial);
    m_configureSent = true;
    xdg_surface::send_configure(serial);
}

uint XdgSurface::sendConfigure()
{
    const uint serial = m_xdgWmBase->m_compositor->nextSerial();
    sendConfigure(serial);
    return serial;
}

void XdgSurface::xdg_surface_get_toplevel(Resource *resource, uint32_t id)
{
    QVERIFY(!m_toplevel);
    QVERIFY(!m_popup);
    m_toplevel = new XdgToplevel(this, id, resource->version());
    emit toplevelCreated(m_toplevel);
}

void XdgSurface::xdg_surface_get_popup(Resource *resource, uint32_t id, wl_resource *parent, wl_resource *positioner)
{
    Q_UNUSED(parent);
    Q_UNUSED(positioner);
    QVERIFY(!m_toplevel);
    QVERIFY(!m_popup);
    m_popup = new XdgPopup(this, id, resource->version());
}

void XdgSurface::xdg_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    bool removed = m_xdgWmBase->m_xdgSurfaces.removeOne(this);
    Q_ASSERT(removed);
    delete this;
}

void XdgSurface::xdg_surface_ack_configure(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    QVERIFY2(m_pendingConfigureSerials.contains(serial), qPrintable(QString::number(serial)));
    m_ackedConfigureSerial = serial;
    while (!m_pendingConfigureSerials.empty()) {
        uint s = m_pendingConfigureSerials.takeFirst();
        if (s == serial)
            return;
    }
}

XdgToplevel::XdgToplevel(XdgSurface *xdgSurface, int id, int version)
    : QtWaylandServer::xdg_toplevel(xdgSurface->resource()->client(), id, version)
    , m_xdgSurface(xdgSurface)
{
}

void XdgToplevel::sendConfigure(const QSize &size, const QVector<uint> &states)
{
    send_configure(size.width(), size.height(), toByteArray(states));
}

uint XdgToplevel::sendCompleteConfigure(const QSize &size, const QVector<uint> &states)
{
    sendConfigure(size, states);
    return m_xdgSurface->sendConfigure();
}

XdgPopup::XdgPopup(XdgSurface *xdgSurface, int id, int version)
    : QtWaylandServer::xdg_popup(xdgSurface->resource()->client(), id, version)
    , m_xdgSurface(xdgSurface)
{
}

void XdgPopup::sendConfigure(const QRect &geometry)
{
    send_configure(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void XdgPopup::xdg_popup_grab(QtWaylandServer::xdg_popup::Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(seat); // TODO: verify correct seat as well
    //TODO: verify no other popup has grabbed
    QVERIFY(!m_grabbed);
    m_grabbed = true;
    m_grabSerial = serial;
}

} // namespace MockCompositor
