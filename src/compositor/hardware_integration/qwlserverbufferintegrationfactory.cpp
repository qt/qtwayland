// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlserverbufferintegrationfactory_p.h"
#include "qwlserverbufferintegrationplugin_p.h"
#include "qwlserverbufferintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qwsbifLoader,
    (QtWaylandServerBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ServerBufferIntegrationFactory::keys()
{
    return qwsbifLoader->keyMap().values();
}

ServerBufferIntegration *ServerBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<ServerBufferIntegration, ServerBufferIntegrationPlugin>(qwsbifLoader(), name, args);
}

}

QT_END_NAMESPACE
