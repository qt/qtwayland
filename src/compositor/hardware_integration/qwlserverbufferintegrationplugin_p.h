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

#ifndef QWAYLANDSERVERBUFFERINTEGRATIONPLUGIN_H
#define QWAYLANDSERVERBUFFERINTEGRATIONPLUGIN_H

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

class ServerBufferIntegration;

#define QtWaylandServerBufferIntegrationFactoryInterface_iid "org.qt-project.Qt.Compositor.QtWaylandServerBufferIntegrationFactoryInterface.5.3"

class Q_WAYLAND_COMPOSITOR_EXPORT ServerBufferIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ServerBufferIntegrationPlugin(QObject *parent = nullptr);
    ~ServerBufferIntegrationPlugin() override;

    virtual ServerBufferIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSERVERBUFFERINTEGRATIONPLUGIN_H
