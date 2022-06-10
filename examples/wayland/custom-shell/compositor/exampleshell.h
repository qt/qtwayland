// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EXAMPLESHELL_H
#define EXAMPLESHELL_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtCore/QSize>

#include <QtWaylandCompositor/QWaylandShellSurface>
#include "qwayland-server-example-shell.h"

class ExampleShellSurface;

//! [ExampleShell]
class ExampleShell
        : public QWaylandCompositorExtensionTemplate<ExampleShell>
        , QtWaylandServer::qt_example_shell
//! [ExampleShell]
{
    Q_OBJECT
public:
    ExampleShell();
    ExampleShell(QWaylandCompositor *compositor);

    void initialize() override;

    using QtWaylandServer::qt_example_shell::interface;
    using QtWaylandServer::qt_example_shell::interfaceName;

protected:
    void example_shell_surface_create(Resource *resource, wl_resource *surfaceResource, uint32_t id) override;

signals:
    void shellSurfaceCreated(ExampleShellSurface *shellSurface);

private:
    QWaylandCompositor *compositor;
};

class ExampleShellSurface :
        public QWaylandShellSurfaceTemplate<ExampleShellSurface>
        , public QtWaylandServer::qt_example_shell_surface
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool minimized READ minimized WRITE setMinimized NOTIFY minimizedChanged)
public:
    ExampleShellSurface(ExampleShell *shell, QWaylandSurface *surface, const QWaylandResource &resource);

    using QtWaylandServer::qt_example_shell_surface::interface;
    using QtWaylandServer::qt_example_shell_surface::interfaceName;
    static QWaylandSurfaceRole *role();
    static ExampleShellSurface *fromResource(::wl_resource *resource);

    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;

    QWaylandSurface *surface() const { return m_surface; }
    const QString &windowTitle() const { return m_windowTitle; }
    bool minimized() const { return m_minimized; }
    void setMinimized(bool newMinimized);

signals:
    void windowTitleChanged();
    void minimizedChanged();

protected:
    void example_shell_surface_destroy_resource(Resource *resource) override;
    void example_shell_surface_destroy(Resource *resource) override;
    void example_shell_surface_set_window_title(Resource *resource, const QString &window_title) override;
    void example_shell_surface_set_minimized(Resource *resource, uint32_t minimized) override;

private:
    static QWaylandSurfaceRole s_role;
    QWaylandSurface *m_surface = nullptr;
    ExampleShell *m_shell = nullptr;
    QString m_windowTitle;
    bool m_minimized = false;
};

//! [declare_extension]
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(ExampleShell)
//! [declare_extension]

#endif // EXAMPLESHELL_H
