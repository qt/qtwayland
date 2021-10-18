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
