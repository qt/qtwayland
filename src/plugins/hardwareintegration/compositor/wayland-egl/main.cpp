// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlclientbufferintegrationfactory_p.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegrationplugin_p.h>
#include <QtWaylandEglCompositorHwIntegration/private/waylandeglclientbufferintegration_p.h>

QT_BEGIN_NAMESPACE

class QWaylandEglClientBufferIntegrationPlugin : public QtWayland::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandClientBufferIntegrationFactoryInterface_iid FILE "wayland-egl.json")
public:
    QtWayland::ClientBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ClientBufferIntegration *QWaylandEglClientBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new WaylandEglClientBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
