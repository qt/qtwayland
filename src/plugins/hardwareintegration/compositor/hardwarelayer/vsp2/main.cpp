// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlhardwarelayerintegrationplugin_p.h>
#include "vsp2hardwarelayerintegration.h"

QT_BEGIN_NAMESPACE

class Vsp2HardwareLayerIntegrationPlugin : public QtWayland::HardwareLayerIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandHardwareLayerIntegrationFactoryInterface_iid FILE "vsp2.json")
public:
    QtWayland::HardwareLayerIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::HardwareLayerIntegration *Vsp2HardwareLayerIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new Vsp2HardwareLayerIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
