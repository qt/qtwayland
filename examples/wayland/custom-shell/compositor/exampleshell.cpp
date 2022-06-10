// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exampleshell.h"
#include "exampleshellintegration.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>

ExampleShell::ExampleShell()
{
}

ExampleShell::ExampleShell(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<ExampleShell>(compositor)
{
    this->compositor = compositor;
}

//! [initialize]
void ExampleShell::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing ExampleShell";
        return;
    }

    init(compositor->display(), 1);
}
//! [initialize]

//! [surface_create]
void ExampleShell::example_shell_surface_create(Resource *resource, wl_resource *surfaceResource, uint32_t id)
{
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    if (!surface->setRole(ExampleShellSurface::role(), resource->handle, QT_EXAMPLE_SHELL_ERROR_ROLE))
        return;

    QWaylandResource shellSurfaceResource(wl_resource_create(resource->client(), &::qt_example_shell_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    auto *shellSurface = new ExampleShellSurface(this, surface, shellSurfaceResource);
    emit shellSurfaceCreated(shellSurface);
}
//! [surface_create]

ExampleShellSurface::ExampleShellSurface(ExampleShell *shell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    m_shell = shell;
    m_surface = surface;
    init(resource.resource());
    setExtensionContainer(surface);
    QWaylandCompositorExtension::initialize();
}

QWaylandSurfaceRole ExampleShellSurface::s_role("qt_example_shell_surface");

/*!
 * Returns the surface role for the ExampleShellSurface.
 */
QWaylandSurfaceRole *ExampleShellSurface::role()
{
    return &s_role;
}

/*!
 * Returns the ExampleShellSurface corresponding to the \a resource.
 */
ExampleShellSurface *ExampleShellSurface::fromResource(wl_resource *resource)
{
    auto *res = Resource::fromResource(resource);
    return static_cast<ExampleShellSurface *>(res->object());
}

QWaylandQuickShellIntegration *ExampleShellSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new ExampleShellIntegration(item);
}

void ExampleShellSurface::example_shell_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void ExampleShellSurface::example_shell_surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void ExampleShellSurface::example_shell_surface_set_window_title(Resource *resource, const QString &window_title)
{
    Q_UNUSED(resource);
    m_windowTitle = window_title;
    emit windowTitleChanged();
}

void ExampleShellSurface::example_shell_surface_set_minimized(Resource *resource, uint32_t minimized)
{
    Q_UNUSED(resource);
    if (m_minimized != minimized) {
        m_minimized = minimized;
        emit minimizedChanged();
    }
}

void ExampleShellSurface::setMinimized(bool newMinimized)
{
    if (m_minimized == newMinimized)
        return;
    m_minimized = newMinimized;
    send_minimize(newMinimized);
    emit minimizedChanged();
}
