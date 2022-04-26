/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd
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

#include "qwaylandshellintegrationfactory_p.h"
#include "qwaylandshellintegrationplugin_p.h"
#include "qwaylandshellintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandShellIntegrationFactoryInterface_iid, QLatin1String("/wayland-shell-integration"), Qt::CaseInsensitive))

QStringList QWaylandShellIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

QWaylandShellIntegration *QWaylandShellIntegrationFactory::create(const QString &name, QWaylandDisplay *display, const QStringList &args)
{
    std::unique_ptr<QWaylandShellIntegration> integration;
    integration.reset(qLoadPlugin<QWaylandShellIntegration, QWaylandShellIntegrationPlugin>(loader(), name, args));

    if (integration && !integration->initialize(display))
        return nullptr;

    return integration.release();
}

}

QT_END_NAMESPACE
