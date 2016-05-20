/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QWAYLANDXDGSHELL_P_H
#define QWAYLANDXDGSHELL_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-shell.h>

#include <QtWaylandCompositor/QWaylandXdgShell>

#include <QtCore/QSet>

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

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShellPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::xdg_shell
{
    Q_DECLARE_PUBLIC(QWaylandXdgShell)
public:
    QWaylandXdgShellPrivate();
    void ping(Resource *resource, uint32_t serial);
    void registerSurface(QWaylandXdgSurface *xdgSurface);
    void unregisterXdgSurface(QWaylandXdgSurface *xdgSurface);
    void registerXdgPopup(QWaylandXdgPopup *xdgPopup);
    void unregisterXdgPopup(QWaylandXdgPopup *xdgPopup);
    static QWaylandXdgShellPrivate *get(QWaylandXdgShell *xdgShell) { return xdgShell->d_func(); }
    bool isValidPopupParent(QWaylandSurface *parentSurface) const;
    QWaylandXdgPopup *topmostPopupForClient(struct wl_client* client) const;

private:
    QSet<uint32_t> m_pings;
    QMultiMap<struct wl_client *, QWaylandXdgSurface *> m_xdgSurfaces;
    QMultiMap<struct wl_client *, QWaylandXdgPopup *> m_xdgPopups;

    QWaylandXdgSurface *xdgSurfaceFromSurface(QWaylandSurface *surface);

    void xdg_shell_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_shell_get_xdg_surface(Resource *resource, uint32_t id,
                                   struct ::wl_resource *surface) Q_DECL_OVERRIDE;
    void xdg_shell_use_unstable_version(Resource *resource, int32_t version) Q_DECL_OVERRIDE;
    void xdg_shell_get_xdg_popup(Resource *resource, uint32_t id, struct ::wl_resource *surface,
                                 struct ::wl_resource *parent, struct ::wl_resource *seat,
                                 uint32_t serial, int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void xdg_shell_pong(Resource *resource, uint32_t serial) Q_DECL_OVERRIDE;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurfacePrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::xdg_surface
{
    Q_DECLARE_PUBLIC(QWaylandXdgSurface)
public:
    QWaylandXdgSurfacePrivate();
    static QWaylandXdgSurfacePrivate *get(QWaylandXdgSurface *xdgSurface) { return xdgSurface->d_func(); }

    struct ConfigureEvent {
        QVector<uint> states;
        QSize size;
        uint serial;
    };

    void handleFocusLost();
    void handleFocusReceived();

private:
    QWaylandXdgShell *m_xdgShell;
    QWaylandSurface *m_surface;
    QWaylandXdgSurface *m_parentSurface;

    QString m_title;
    QString m_appId;
    QRect m_windowGeometry;
    bool m_unsetWindowGeometry;

    QList<ConfigureEvent> m_pendingConfigures;
    ConfigureEvent m_lastAckedConfigure;
    ConfigureEvent lastSentConfigure() const { return m_pendingConfigures.empty() ? m_lastAckedConfigure : m_pendingConfigures.first(); }

    void xdg_surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void xdg_surface_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_surface_move(Resource *resource, struct ::wl_resource *seat,
                          uint32_t serial) Q_DECL_OVERRIDE;
    void xdg_surface_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial,
                            uint32_t edges) Q_DECL_OVERRIDE;
    void xdg_surface_set_maximized(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_surface_unset_maximized(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_surface_set_fullscreen(Resource *resource,
                                    struct ::wl_resource *output) Q_DECL_OVERRIDE;
    void xdg_surface_unset_fullscreen(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_surface_set_minimized(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_surface_set_parent(Resource *resource, struct ::wl_resource *parent) Q_DECL_OVERRIDE;
    void xdg_surface_set_app_id(Resource *resource, const QString &app_id) Q_DECL_OVERRIDE;
    void xdg_surface_show_window_menu(Resource *resource, struct ::wl_resource *seat,
                                      uint32_t serial, int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) Q_DECL_OVERRIDE;
    void xdg_surface_set_title(Resource *resource, const QString &title) Q_DECL_OVERRIDE;
    void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y,
                                         int32_t width, int32_t height) Q_DECL_OVERRIDE;

    static QWaylandSurfaceRole s_role;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopupPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::xdg_popup
{
    Q_DECLARE_PUBLIC(QWaylandXdgPopup)

public:
    QWaylandXdgPopupPrivate();
    static QWaylandXdgPopupPrivate *get(QWaylandXdgPopup *xdgPopup) { return xdgPopup->d_func(); }

private:
    QWaylandSurface *m_surface;
    QWaylandSurface *m_parentSurface;
    QWaylandXdgShell *m_xdgShell;

    void xdg_popup_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void xdg_popup_destroy(xdg_popup::Resource *resource) Q_DECL_OVERRIDE;

    static QWaylandSurfaceRole s_role;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELL_P_H
