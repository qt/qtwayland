// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include "dmabufserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class DmaBufServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "dmabuf-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ServerBufferIntegration *DmaBufServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new DmaBufServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
