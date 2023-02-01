// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlhardwarelayerintegrationfactory_p.h"
#include "qwlhardwarelayerintegrationplugin_p.h"
#include "qwlhardwarelayerintegration_p.h"

#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qwhlifLoader,
    (QtWaylandHardwareLayerIntegrationFactoryInterface_iid, QLatin1String("/wayland-hardware-layer-integration"), Qt::CaseInsensitive))

QStringList HardwareLayerIntegrationFactory::keys()
{
    return qwhlifLoader->keyMap().values();
}

HardwareLayerIntegration *HardwareLayerIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<HardwareLayerIntegration, HardwareLayerIntegrationPlugin>(qwhlifLoader(), name, args);
}

}

QT_END_NAMESPACE
