/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include "shmserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class ShmServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "shm-emulation-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QtWayland::ServerBufferIntegration *ShmServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new ShmServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
