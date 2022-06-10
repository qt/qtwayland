// Copyright (C) 2016 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWaylandCompositor/private/qwlserverbufferintegrationplugin_p.h>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>
#include "libhybriseglserverbufferintegration.h"

QT_BEGIN_NAMESPACE

class LibHybrisEglServerBufferIntegrationPlugin : public QtWayland::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtWaylandServerBufferIntegrationFactoryInterface_iid FILE "libhybris-egl-server.json")
public:
    QtWayland::ServerBufferIntegration *create(const QString&, const QStringList&);
};

QtWayland::ServerBufferIntegration *LibHybrisEglServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new LibHybrisEglServerBufferIntegration();
}

QT_END_NAMESPACE

#include "main.moc"
