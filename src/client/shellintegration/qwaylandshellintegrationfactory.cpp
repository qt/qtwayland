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
****************************************************************************/

#include "qwaylandshellintegrationfactory_p.h"
#include "qwaylandshellintegrationplugin_p.h"
#include "qwaylandshellintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

#if QT_CONFIG(library)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandShellIntegrationFactoryInterface_iid, QLatin1String("/wayland-shell-integration"), Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader,
                          (QWaylandShellIntegrationFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QStringList QWaylandShellIntegrationFactory::keys(const QString &pluginPath)
{
#if QT_CONFIG(library)
    QStringList list;
    if (!pluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(pluginPath);
        list = directLoader()->keyMap().values();
        if (!list.isEmpty()) {
            const QString postFix = QStringLiteral(" (from ")
                                    + QDir::toNativeSeparators(pluginPath)
                                    + QLatin1Char(')');
            const QStringList::iterator end = list.end();
            for (QStringList::iterator it = list.begin(); it != end; ++it)
                (*it).append(postFix);
        }
    }
    list.append(loader()->keyMap().values());
    return list;
#else
    return QStringList();
#endif
}

QWaylandShellIntegration *QWaylandShellIntegrationFactory::create(const QString &name, QWaylandDisplay *display, const QStringList &args, const QString &pluginPath)
{
#if QT_CONFIG(library)
    QScopedPointer<QWaylandShellIntegration> integration;

    // Try loading the plugin from platformPluginPath first:
    if (!pluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(pluginPath);
        integration.reset(qLoadPlugin<QWaylandShellIntegration, QWaylandShellIntegrationPlugin>(directLoader(), name, args));
    }
    if (!integration)
        integration.reset(qLoadPlugin<QWaylandShellIntegration, QWaylandShellIntegrationPlugin>(loader(), name, args));
#endif

    if (integration && !integration->initialize(display))
        return nullptr;

    return integration.take();
}

}

QT_END_NAMESPACE
