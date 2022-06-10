// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include "drmeglserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class DrmEglServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "drm-egl-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ServerBufferIntegration *DrmEglServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new DrmEglServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
