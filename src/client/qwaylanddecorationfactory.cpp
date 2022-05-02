/****************************************************************************
**
** Copyright (C) 2016 Robin Burchell <robin.burchell@viroteck.net>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylanddecorationfactory_p.h"
#include "qwaylanddecorationplugin_p.h"

#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWaylandDecorationFactoryInterface_iid, QLatin1String("/wayland-decoration-client"), Qt::CaseInsensitive))

QStringList QWaylandDecorationFactory::keys()
{
    return loader->keyMap().values();
}

QWaylandAbstractDecoration *QWaylandDecorationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<QWaylandAbstractDecoration, QWaylandDecorationPlugin>(loader(), name, args);
}

}

QT_END_NAMESPACE
