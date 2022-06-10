// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EXAMPLESHELLINTEGRATION_H
#define EXAMPLESHELLINTEGRATION_H
#include <QtWaylandClient/private/qwaylandclientshellapi_p.h>
#include "qwayland-example-shell.h"

using namespace QtWaylandClient;

//! [shell-integration]
class Q_WAYLANDCLIENT_EXPORT ExampleShellIntegration
        : public QWaylandShellIntegrationTemplate<ExampleShellIntegration>
        , public QtWayland::qt_example_shell
{
public:
    ExampleShellIntegration();

    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;
};
//! [shell-integration]

#endif // EXAMPLESHELLINTEGRATION_H
