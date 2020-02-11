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

#include "mockcompositor.h"

#include <qwayland-server-xdg-decoration-unstable-v1.h>

#include <QtGui/QRasterWindow>
#include <QtGui/QOpenGLWindow>
#include <QtGui/QClipboard>
#include <QtCore/private/qcore_unix_p.h>

#include <fcntl.h>

using namespace MockCompositor;

constexpr int xdgDecorationVersion = 1; // protocol VERSION, not the name suffix (_v1)

class XdgDecorationManagerV1;
class XdgToplevelDecorationV1 : public QObject, public QtWaylandServer::zxdg_toplevel_decoration_v1
{
    Q_OBJECT
public:
    explicit XdgToplevelDecorationV1(XdgDecorationManagerV1 *manager, XdgToplevel *toplevel, int id, int version)
        : zxdg_toplevel_decoration_v1(toplevel->resource()->client(), id, version)
        , m_manager(manager)
        , m_toplevel(toplevel)
    {
    }
    void sendConfigure(mode mode)
    {
        if (!m_configureSent) {
            // Attaching buffers before the configure is a protocol error
            QVERIFY(!m_toplevel->surface()->m_pending.buffer);
            QVERIFY(!m_toplevel->surface()->m_committed.buffer);
        }
        send_configure(mode);
        m_configureSent = true;
    }
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override
    {
        Q_UNUSED(resource);
        m_unsetModeRequested = false;
        m_requestedMode = XdgToplevelDecorationV1::mode(mode);
    }
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override
    {
        Q_UNUSED(resource);
        m_unsetModeRequested = true;
        m_requestedMode = mode(0);
    }
    XdgDecorationManagerV1 *m_manager = nullptr;
    XdgToplevel *m_toplevel = nullptr;
    mode m_requestedMode = mode(0);
    bool m_unsetModeRequested = false;
    bool m_configureSent = false;
};

class XdgDecorationManagerV1 : public Global, public QtWaylandServer::zxdg_decoration_manager_v1
{
    Q_OBJECT
public:
    explicit XdgDecorationManagerV1(CoreCompositor *compositor, int version = 1)
        : QtWaylandServer::zxdg_decoration_manager_v1(compositor->m_display, version)
        , m_version(version)
    {}
    bool isClean() override { return m_decorations.empty(); }
    XdgToplevelDecorationV1 *decorationFor(XdgToplevel *toplevel)
    {
        return m_decorations.value(toplevel, nullptr);
    }

    int m_version = 1; // TODO: Remove on libwayland upgrade
    QMap<XdgToplevel *, XdgToplevelDecorationV1 *> m_decorations;

protected:
    void zxdg_decoration_manager_v1_destroy(Resource *resource) override
    {
        //TODO: Should the decorations be destroyed at this point?
        wl_resource_destroy(resource->handle);
    }

    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint32_t id, ::wl_resource *toplevelResource) override
    {
        auto *toplevel = fromResource<XdgToplevel>(toplevelResource);
        QVERIFY(toplevel);
        QVERIFY(!decorationFor(toplevel));

        // Attaching buffers before the configure is a protocol error
        QVERIFY(!toplevel->surface()->m_pending.buffer);
        QVERIFY(!toplevel->surface()->m_committed.buffer);

        m_decorations[toplevel] = new XdgToplevelDecorationV1(this, toplevel, id, resource->version());
    }
};

void XdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy_resource(QtWaylandServer::zxdg_toplevel_decoration_v1::Resource *resource)
{
    Q_UNUSED(resource);
    int removed = m_manager->m_decorations.remove(m_toplevel);
    Q_ASSERT(removed == 1);
    delete this;
}

class XdgDecorationCompositor : public DefaultCompositor {
public:
    explicit XdgDecorationCompositor()
    {
        exec([this] {
            m_config.autoConfigure = true;
            add<XdgDecorationManagerV1>(xdgDecorationVersion);
        });
    }
    XdgToplevelDecorationV1 *toplevelDecoration(int i = 0) {
        return get<XdgDecorationManagerV1>()->decorationFor(xdgToplevel(i));
    }
};

class tst_xdgdecorationv1 : public QObject, private XdgDecorationCompositor
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void clientSidePreferredByCompositor();
    void initialFramelessWindowHint();
    void delayedFramelessWindowHint();
};

void tst_xdgdecorationv1::initTestCase()
{
    if (qEnvironmentVariableIntValue("QT_WAYLAND_DISABLE_WINDOWDECORATION"))
        QSKIP("This test doesn't make sense when QT_WAYLAND_DISABLE_WINDOWDECORATION is set in the environment");
}

void tst_xdgdecorationv1::clientSidePreferredByCompositor()
{
    QRasterWindow window;
    window.show();
    QCOMPOSITOR_TRY_COMPARE(get<XdgDecorationManagerV1>()->resourceMap().size(), 1);
    QCOMPOSITOR_TRY_COMPARE(get<XdgDecorationManagerV1>()->resourceMap().first()->version(), xdgDecorationVersion);
    QCOMPOSITOR_TRY_VERIFY(toplevelDecoration()); // The client creates a toplevel object

    // Check that we don't assume decorations before the server has configured them
    QVERIFY(window.frameMargins().isNull());

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    QCOMPOSITOR_TRY_VERIFY(toplevelDecoration()->m_unsetModeRequested);
    QVERIFY(window.frameMargins().isNull()); // We're still waiting for a configure
    exec([=] {
        toplevelDecoration()->sendConfigure(XdgToplevelDecorationV1::mode_client_side);
        xdgToplevel()->sendCompleteConfigure();
    });
    QTRY_VERIFY(!window.frameMargins().isNull());
}

void tst_xdgdecorationv1::initialFramelessWindowHint()
{
    QRasterWindow window;
    window.setFlag(Qt::FramelessWindowHint, true);
    window.show();
    QCOMPOSITOR_TRY_COMPARE(get<XdgDecorationManagerV1>()->resourceMap().size(), 1);
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=]{
        xdgToplevel()->sendCompleteConfigure();
    });
    QCOMPOSITOR_TRY_VERIFY(xdgSurface()->m_committedConfigureSerial);

    // The client should not have create a decoration object, because that allows the compositor
    // to override our decision and add server side decorations to our window.
    QCOMPOSITOR_TRY_VERIFY(!toplevelDecoration());
}

void tst_xdgdecorationv1::delayedFramelessWindowHint()
{
    QRasterWindow window;
    window.show();
    QCOMPOSITOR_TRY_COMPARE(get<XdgDecorationManagerV1>()->resourceMap().size(), 1);
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=]{
        xdgToplevel()->sendCompleteConfigure();
    });
    QCOMPOSITOR_TRY_VERIFY(xdgSurface()->m_committedConfigureSerial);
    QCOMPOSITOR_TRY_VERIFY(toplevelDecoration());

    window.setFlag(Qt::FramelessWindowHint, true);

    // The client should now destroy the decoration object, so the compositor is no longer
    // able to force window decorations
    QCOMPOSITOR_TRY_VERIFY(!toplevelDecoration());
}

QCOMPOSITOR_TEST_MAIN(tst_xdgdecorationv1)
#include "tst_xdgdecorationv1.moc"
