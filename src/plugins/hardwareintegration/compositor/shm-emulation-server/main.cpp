// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include "shmserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class ShmServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "shm-emulation-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ServerBufferIntegration *ShmServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new ShmServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
