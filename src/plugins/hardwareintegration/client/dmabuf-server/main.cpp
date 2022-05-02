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

#include <QtWaylandClient/private/qwaylandserverbufferintegrationplugin_p.h>
#include "dmabufserverbufferintegration.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class DmaBufServerBufferPlugin : public QWaylandServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandServerBufferIntegrationFactoryInterface_iid FILE "dmabuf-server.json")
public:
    QWaylandServerBufferIntegration *create(const QString&, const QStringList&) override;
};

QWaylandServerBufferIntegration *DmaBufServerBufferPlugin::create(const QString&, const QStringList&)
{
    return new DmaBufServerBufferIntegration();
}

}

QT_END_NAMESPACE

#include "main.moc"
