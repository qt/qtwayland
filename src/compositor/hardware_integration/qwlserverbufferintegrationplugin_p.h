// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class ServerBufferIntegration;

#define QtWaylandServerBufferIntegrationFactoryInterface_iid "org.qt-project.Qt.Compositor.QtWaylandServerBufferIntegrationFactoryInterface.5.3"

class Q_WAYLANDCOMPOSITOR_EXPORT ServerBufferIntegrationPlugin : public QObject
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
