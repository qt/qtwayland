/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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
**
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

#include "qwaylandserverbufferintegrationfactory_p.h"
#include "qwaylandserverbufferintegrationplugin_p.h"

#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandServerBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-client"), Qt::CaseInsensitive))

QStringList QWaylandServerBufferIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

QWaylandServerBufferIntegration *QWaylandServerBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<QWaylandServerBufferIntegration, QWaylandServerBufferIntegrationPlugin>(loader(), name, args);
}

}

QT_END_NAMESPACE
