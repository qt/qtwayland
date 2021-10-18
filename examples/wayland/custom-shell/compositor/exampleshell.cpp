/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
