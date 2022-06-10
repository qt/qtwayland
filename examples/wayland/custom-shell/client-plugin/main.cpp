// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exampleshellintegration.h"

//! [include]
#include <QtWaylandClient/private/qwaylandclientshellapi_p.h>
//! [include]

#include "qwayland-example-shell.h"

using namespace QtWaylandClient;

//! [plugin]
class QWaylandExampleShellIntegrationPlugin : public QWaylandShellIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandShellIntegrationFactoryInterface_iid FILE "example-shell.json")

public:
    QWaylandShellIntegration *create(const QString &key, const QStringList &paramList) override;
};

QWaylandShellIntegration *QWaylandExampleShellIntegrationPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key);
    Q_UNUSED(paramList);
    return new ExampleShellIntegration();
}
//! [plugin]

#include "main.moc"
