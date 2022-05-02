/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include "qwlserverbufferintegrationfactory_p.h"
#include "qwlserverbufferintegrationplugin_p.h"
#include "qwlserverbufferintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QtWaylandServerBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ServerBufferIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

ServerBufferIntegration *ServerBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<ServerBufferIntegration, ServerBufferIntegrationPlugin>(loader(), name, args);
}

}

QT_END_NAMESPACE
