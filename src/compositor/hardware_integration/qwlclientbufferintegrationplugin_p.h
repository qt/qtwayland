/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef QWAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H
#define QWAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H

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

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class ClientBufferIntegration;

#define QtWaylandClientBufferIntegrationFactoryInterface_iid "org.qt-project.Qt.WaylandCompositor.QtWaylandClientBufferIntegrationFactoryInterface.5.3"

class Q_WAYLAND_COMPOSITOR_EXPORT ClientBufferIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ClientBufferIntegrationPlugin(QObject *parent = nullptr);
    ~ClientBufferIntegrationPlugin() override;

    virtual ClientBufferIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H
