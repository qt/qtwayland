// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qpa/qplatformintegrationplugin.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "qwayland-generic.json")
public:
    QPlatformIntegration *create(const QString&, const QStringList&) override;
};

QPlatformIntegration *QWaylandIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    auto *integration =  new QWaylandIntegration();

    if (!integration->init()) {
        delete integration;
        integration = nullptr;
    }

    return integration;
}

}

QT_END_NAMESPACE

#include "main.moc"
