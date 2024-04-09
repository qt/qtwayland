// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDWLSHELL_P_H
#define QWAYLANDWLSHELL_P_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandsurface.h>
#include <QtWaylandCompositor/private/qwaylandshellsurface_p.h>
#include <QtWaylandCompositor/private/qwaylandshell_p.h>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/QWaylandSeat>

#include <wayland-server-core.h>
#include <QHash>
#include <QPoint>
#include <QSet>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <QtCore/qpointer.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandWlShellPrivate
                                        : public QWaylandShellPrivate
                                        , public QtWaylandServer::wl_shell
{
    Q_DECLARE_PUBLIC(QWaylandWlShell)
public:
    QWaylandWlShellPrivate();

    void unregisterShellSurface(QWaylandWlShellSurface *shellSurface);

    static QWaylandWlShellPrivate *get(QWaylandWlShell *shell) { return shell->d_func(); }

protected:
    void shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) override;

    QList<QWaylandWlShellSurface *> m_shellSurfaces;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandWlShellSurfacePrivate
                                        : public QWaylandShellSurfacePrivate
                                        , public QtWaylandServer::wl_shell_surface
{
    Q_DECLARE_PUBLIC(QWaylandWlShellSurface)
public:
    QWaylandWlShellSurfacePrivate();
    ~QWaylandWlShellSurfacePrivate() override;

    static QWaylandWlShellSurfacePrivate *get(QWaylandWlShellSurface *surface) { return surface->d_func(); }

    void ping(uint32_t serial);

    void setWindowType(Qt::WindowType windowType);

private:
    QWaylandWlShell *m_shell = nullptr;
    QPointer<QWaylandSurface> m_surface;

    QSet<uint32_t> m_pings;

    QString m_title;
    QString m_className;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void shell_surface_destroy_resource(Resource *resource) override;

    void shell_surface_move(Resource *resource,
                            struct wl_resource *input_device_super,
                            uint32_t time) override;
    void shell_surface_resize(Resource *resource,
                              struct wl_resource *input_device,
                              uint32_t time,
                              uint32_t edges) override;
    void shell_surface_set_toplevel(Resource *resource) override;
    void shell_surface_set_transient(Resource *resource,
                                     struct wl_resource *parent_surface_resource,
                                     int x,
                                     int y,
                                     uint32_t flags) override;
    void shell_surface_set_fullscreen(Resource *resource,
                                      uint32_t method,
                                      uint32_t framerate,
                                      struct wl_resource *output_resource) override;
    void shell_surface_set_popup(Resource *resource,
                                 struct wl_resource *input_device,
                                 uint32_t time,
                                 struct wl_resource *parent,
                                 int32_t x,
                                 int32_t y,
                                 uint32_t flags) override;
    void shell_surface_set_maximized(Resource *resource,
                                     struct wl_resource *output_resource) override;
    void shell_surface_pong(Resource *resource,
                            uint32_t serial) override;
    void shell_surface_set_title(Resource *resource,
                                 const QString &title) override;
    void shell_surface_set_class(Resource *resource,
                                 const QString &class_) override;

    static QWaylandSurfaceRole s_role;
};

QT_END_NAMESPACE

#endif // QWAYLANDWLSHELL_P_H
