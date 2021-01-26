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
****************************************************************************/

#ifndef QWAYLANDDECORATIONPLUGIN_H
#define QWAYLANDDECORATIONPLUGIN_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandAbstractDecoration;

#define QWaylandDecorationFactoryInterface_iid "org.qt-project.Qt.WaylandClient.QWaylandDecorationFactoryInterface.5.4"

class Q_WAYLAND_CLIENT_EXPORT QWaylandDecorationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit QWaylandDecorationPlugin(QObject *parent = nullptr);
    ~QWaylandDecorationPlugin() override;

    virtual QWaylandAbstractDecoration *create(const QString &key, const QStringList &paramList) = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDDECORATIONPLUGIN_H
