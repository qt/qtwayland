// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlclientbufferintegrationfactory_p.h"
#include "qwlclientbufferintegrationplugin_p.h"
#include "qwlclientbufferintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qwbifiLoader,
    (QtWaylandClientBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ClientBufferIntegrationFactory::keys()
{
    return qwbifiLoader->keyMap().values();
}

ClientBufferIntegration *ClientBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<ClientBufferIntegration, ClientBufferIntegrationPlugin>(qwbifiLoader(), name, args);
}

}

QT_END_NAMESPACE
