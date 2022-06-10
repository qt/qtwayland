// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlclientbufferintegrationfactory_p.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegrationplugin_p.h>
#include "waylandeglstreamintegration.h"

QT_BEGIN_NAMESPACE

class QWaylandEglStreamClientBufferIntegrationPlugin : public QtWayland::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandClientBufferIntegrationFactoryInterface_iid FILE "wayland-eglstream-controller.json")
public:
    QtWayland::ClientBufferIntegration *create(const QString& key, const QStringList& paramList) override;
};

QtWayland::ClientBufferIntegration *QWaylandEglStreamClientBufferIntegrationPlugin::create(const QString& key, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(key);
    return new WaylandEglStreamClientBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
