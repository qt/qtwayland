// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGDECORATIONV1_P_H
#define QWAYLANDXDGDECORATIONV1_P_H

#include "qwaylandxdgdecorationv1.h"

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-decoration-unstable-v1.h>

#include <QtWaylandCompositor/QWaylandXdgToplevel>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QWaylandXdgToplevel;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgDecorationManagerV1Private
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::zxdg_decoration_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandXdgDecorationManagerV1)
public:
    using DecorationMode = QWaylandXdgToplevel::DecorationMode;
    explicit QWaylandXdgDecorationManagerV1Private() {}

protected:
    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint id, ::wl_resource *toplevelResource) override;

private:
    DecorationMode m_preferredMode = DecorationMode::ClientSideDecoration;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgToplevelDecorationV1
        : public QtWaylandServer::zxdg_toplevel_decoration_v1
{
public:
    using DecorationMode = QWaylandXdgToplevel::DecorationMode;
    explicit QWaylandXdgToplevelDecorationV1(QWaylandXdgToplevel *toplevel,
                                             QWaylandXdgDecorationManagerV1 *manager,
                                             wl_client *client, int id);
    ~QWaylandXdgToplevelDecorationV1() override;

    DecorationMode configuredMode() const { return DecorationMode(m_configuredMode); }
    void sendConfigure(DecorationMode mode);

protected:
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override;
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override;

private:
    void handleClientPreferredModeChanged();

    QWaylandXdgToplevel *m_toplevel = nullptr;
    QWaylandXdgDecorationManagerV1 *m_manager = nullptr;
    uint m_configuredMode = 0;
    uint m_clientPreferredMode = 0;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGDECORATIONV1_P_H
