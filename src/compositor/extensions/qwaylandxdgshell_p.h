// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGSHELL_P_H
#define QWAYLANDXDGSHELL_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwaylandshell_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-shell.h>

#include <QtWaylandCompositor/QWaylandXdgShell>

#include <QtWaylandCompositor/private/qwaylandxdgdecorationv1_p.h>

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

struct Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgPositionerData {
    QSize size;
    QRect anchorRect;
    Qt::Edges anchorEdges = {};
    Qt::Edges gravityEdges = {};
    uint constraintAdjustments = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    QPoint offset;
    QWaylandXdgPositionerData();
    bool isComplete() const;
    QPoint anchorPoint() const;
    QPoint unconstrainedPosition() const;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgShellPrivate
        : public QWaylandShellPrivate
        , public QtWaylandServer::xdg_wm_base
{
    Q_DECLARE_PUBLIC(QWaylandXdgShell)
public:
    QWaylandXdgShellPrivate();
    void ping(Resource *resource, uint32_t serial);
    void registerXdgSurface(QWaylandXdgSurface *xdgSurface);
    void unregisterXdgSurface(QWaylandXdgSurface *xdgSurface);
    static QWaylandXdgShellPrivate *get(QWaylandXdgShell *xdgShell) { return xdgShell->d_func(); }

    QSet<uint32_t> m_pings;
    QMultiMap<struct wl_client *, QWaylandXdgSurface *> m_xdgSurfaces;

    QWaylandXdgSurface *xdgSurfaceFromSurface(QWaylandSurface *surface);

protected:
    void xdg_wm_base_destroy(Resource *resource) override;
    void xdg_wm_base_create_positioner(Resource *resource, uint32_t id) override;
    void xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id,
                                     struct ::wl_resource *surface) override;
    void xdg_wm_base_pong(Resource *resource, uint32_t serial) override;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgSurfacePrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::xdg_surface
{
    Q_DECLARE_PUBLIC(QWaylandXdgSurface)
public:
    QWaylandXdgSurfacePrivate();
    void setWindowType(Qt::WindowType windowType);
    void handleFocusLost();
    void handleFocusReceived();
    static QWaylandXdgSurfacePrivate *get(QWaylandXdgSurface *xdgSurface) { return xdgSurface->d_func(); }

    QRect calculateFallbackWindowGeometry() const;
    void updateFallbackWindowGeometry();

private:
    QWaylandXdgShell *m_xdgShell = nullptr;
    QWaylandSurface *m_surface = nullptr;

    QWaylandXdgToplevel *m_toplevel = nullptr;
    QWaylandXdgPopup *m_popup = nullptr;
    QRect m_windowGeometry;
    bool m_unsetWindowGeometry = true;
    QMargins m_windowMargins;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void xdg_surface_destroy_resource(Resource *resource) override;
    void xdg_surface_destroy(Resource *resource) override;
    void xdg_surface_get_toplevel(Resource *resource, uint32_t id) override;
    void xdg_surface_get_popup(Resource *resource, uint32_t id, struct ::wl_resource *parent, struct ::wl_resource *positioner) override;
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) override;
    void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgToplevelPrivate : public QObjectPrivate, public QtWaylandServer::xdg_toplevel
{
    Q_DECLARE_PUBLIC(QWaylandXdgToplevel)
public:
    struct ConfigureEvent {
        ConfigureEvent() = default;
        ConfigureEvent(const QList<QWaylandXdgToplevel::State>
                       &incomingStates,
                       const QSize &incomingSize, uint incomingSerial)
        : states(incomingStates), size(incomingSize), serial(incomingSerial)
        { }
        QList<QWaylandXdgToplevel::State> states;
        QSize size = {0, 0};
        uint serial = 0;
    };

    QWaylandXdgToplevelPrivate(QWaylandXdgSurface *xdgSurface, const QWaylandResource& resource);
    ConfigureEvent lastSentConfigure() const { return m_pendingConfigures.empty() ? m_lastAckedConfigure : m_pendingConfigures.last(); }
    void handleAckConfigure(uint serial); //TODO: move?
    void handleFocusLost();
    void handleFocusReceived();

    static QWaylandXdgToplevelPrivate *get(QWaylandXdgToplevel *toplevel) { return toplevel->d_func(); }
    static Qt::Edges convertToEdges(resize_edge edge);

protected:

    void xdg_toplevel_destroy_resource(Resource *resource) override;

    void xdg_toplevel_destroy(Resource *resource) override;
    void xdg_toplevel_set_parent(Resource *resource, struct ::wl_resource *parent) override;
    void xdg_toplevel_set_title(Resource *resource, const QString &title) override;
    void xdg_toplevel_set_app_id(Resource *resource, const QString &app_id) override;
    void xdg_toplevel_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void xdg_toplevel_move(Resource *resource, struct ::wl_resource *seatResource, uint32_t serial) override;
    void xdg_toplevel_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void xdg_toplevel_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_maximized(Resource *resource) override;
    void xdg_toplevel_unset_maximized(Resource *resource) override;
    void xdg_toplevel_set_fullscreen(Resource *resource, struct ::wl_resource *output) override;
    void xdg_toplevel_unset_fullscreen(Resource *resource) override;
    void xdg_toplevel_set_minimized(Resource *resource) override;

public:
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgToplevel *m_parentToplevel = nullptr;
    QList<ConfigureEvent> m_pendingConfigures;
    ConfigureEvent m_lastAckedConfigure;
    QString m_title;
    QString m_appId;
    QSize m_maxSize;
    QSize m_minSize = {0, 0};
    QWaylandXdgToplevelDecorationV1 *m_decoration = nullptr;

    static QWaylandSurfaceRole s_role;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgPopupPrivate : public QObjectPrivate, public QtWaylandServer::xdg_popup
{
    Q_DECLARE_PUBLIC(QWaylandXdgPopup)
public:
    struct ConfigureEvent {
        QRect geometry;
        uint serial;
    };

    QWaylandXdgPopupPrivate(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parentXdgSurface,
                            QWaylandXdgPositioner *positioner, const QWaylandResource& resource);

    void handleAckConfigure(uint serial);

    static QWaylandXdgPopupPrivate *get(QWaylandXdgPopup *popup) { return popup->d_func(); }

    static QWaylandSurfaceRole s_role;

private:
    uint sendConfigure(const QRect &geometry);

protected:
    void xdg_popup_destroy(Resource *resource) override;
    void xdg_popup_grab(Resource *resource, struct ::wl_resource *seat, uint32_t serial) override;

private:
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgSurface *m_parentXdgSurface = nullptr;
    QWaylandXdgPositionerData m_positionerData;
    QRect m_geometry;
    QList<ConfigureEvent> m_pendingConfigures;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgPositioner : public QtWaylandServer::xdg_positioner
{
public:
    QWaylandXdgPositioner(const QWaylandResource& resource);
    static QWaylandXdgPositioner *fromResource(wl_resource *resource);
    static Qt::Edges convertToEdges(anchor anchor);
    static Qt::Edges convertToEdges(gravity gravity);

protected:
    void xdg_positioner_destroy_resource(Resource *resource) override; //TODO: do something special here?

    void xdg_positioner_destroy(Resource *resource) override;
    void xdg_positioner_set_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor(Resource *resource, uint32_t anchor) override;
    void xdg_positioner_set_gravity(Resource *resource, uint32_t gravity) override;
    void xdg_positioner_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void xdg_positioner_set_offset(Resource *resource, int32_t x, int32_t y) override;

public:
    QWaylandXdgPositionerData m_data;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELL_P_H
