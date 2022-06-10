// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EXAMPLESHELLSURFACE_H
#define EXAMPLESHELLSURFACE_H

#include <QtWaylandClient/private/qwaylandclientshellapi_p.h>
#include "qwayland-example-shell.h"

using namespace QtWaylandClient;

//! [ExampleShellSurface]
class ExampleShellSurface : public QWaylandShellSurface
        , public QtWayland::qt_example_shell_surface
//! [ExampleShellSurface]
{
public:
    ExampleShellSurface(struct ::qt_example_shell_surface *shell_surface, QWaylandWindow *window);
    ~ExampleShellSurface() override;

//! [virtuals]
    bool wantsDecorations() const override;
    void setTitle(const QString &) override;
    void requestWindowStates(Qt::WindowStates states) override;
    void applyConfigure() override;
//! [virtuals]

protected:
//! [events]
    void example_shell_surface_minimize(uint32_t minimized) override;
//! [events]

private:
    Qt::WindowStates m_pendingStates;
    bool m_stateChanged = false;
};

#endif // EXAMPLESHELLSURFACE_H
