// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlserverbufferintegrationplugin_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

ServerBufferIntegrationPlugin::ServerBufferIntegrationPlugin(QObject *parent) :
    QObject(parent)
{
}

ServerBufferIntegrationPlugin::~ServerBufferIntegrationPlugin()
{
}

}

QT_END_NAMESPACE

#include "moc_qwlserverbufferintegrationplugin_p.cpp"

