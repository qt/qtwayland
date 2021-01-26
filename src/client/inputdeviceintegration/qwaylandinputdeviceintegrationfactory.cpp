/****************************************************************************
**
** Copyright (C) 2016 LG Electronics Ltd
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

#include "qwaylandinputdeviceintegrationfactory_p.h"
#include "qwaylandinputdeviceintegrationplugin_p.h"
#include "qwaylandinputdeviceintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

#if QT_CONFIG(library)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandInputDeviceIntegrationFactoryInterface_iid, QLatin1String("/wayland-inputdevice-integration"), Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader,
                          (QWaylandInputDeviceIntegrationFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QStringList QWaylandInputDeviceIntegrationFactory::keys(const QString &pluginPath)
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

QWaylandInputDeviceIntegration *QWaylandInputDeviceIntegrationFactory::create(const QString &name, const QStringList &args, const QString &pluginPath)
{
#if QT_CONFIG(library)
    // Try loading the plugin from platformPluginPath first:
    if (!pluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(pluginPath);
        if (QWaylandInputDeviceIntegration *ret = qLoadPlugin<QWaylandInputDeviceIntegration, QWaylandInputDeviceIntegrationPlugin>(directLoader(), name, args))
            return ret;
    }
    if (QWaylandInputDeviceIntegration *ret = qLoadPlugin<QWaylandInputDeviceIntegration, QWaylandInputDeviceIntegrationPlugin>(loader(), name, args))
        return ret;
#endif
    return nullptr;
}

}

QT_END_NAMESPACE
