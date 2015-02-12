/****************************************************************************
**
** Copyright (C) 2014 LG Electronics Ltd
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
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

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandInputDeviceIntegrationFactoryInterface_iid, QLatin1String("/wayland-inputdevice-integration"), Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader,
                          (QWaylandInputDeviceIntegrationFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QStringList QWaylandInputDeviceIntegrationFactory::keys(const QString &pluginPath)
{
#ifndef QT_NO_LIBRARY
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
#ifndef QT_NO_LIBRARY
    // Try loading the plugin from platformPluginPath first:
    if (!pluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(pluginPath);
        if (QWaylandInputDeviceIntegration *ret = qLoadPlugin1<QWaylandInputDeviceIntegration, QWaylandInputDeviceIntegrationPlugin>(directLoader(), name, args))
            return ret;
    }
    if (QWaylandInputDeviceIntegration *ret = qLoadPlugin1<QWaylandInputDeviceIntegration, QWaylandInputDeviceIntegrationPlugin>(loader(), name, args))
        return ret;
#endif
    return Q_NULLPTR;
}

}

QT_END_NAMESPACE
