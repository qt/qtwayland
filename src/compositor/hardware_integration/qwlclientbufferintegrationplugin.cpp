// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlclientbufferintegrationplugin_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

ClientBufferIntegrationPlugin::ClientBufferIntegrationPlugin(QObject *parent) :
    QObject(parent)
{
}

ClientBufferIntegrationPlugin::~ClientBufferIntegrationPlugin()
{
}

}

QT_END_NAMESPACE

#include "moc_qwlclientbufferintegrationplugin_p.cpp"
