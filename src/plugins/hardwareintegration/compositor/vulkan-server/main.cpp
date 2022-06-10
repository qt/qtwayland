// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include "vulkanserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class VulkanServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "vulkan-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ServerBufferIntegration *VulkanServerBufferIntegrationPlugin::create(const QString& key, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(key);
    return new VulkanServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
