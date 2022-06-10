// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlhardwarelayerintegrationplugin_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

HardwareLayerIntegrationPlugin::HardwareLayerIntegrationPlugin(QObject *parent) :
    QObject(parent)
{
}

HardwareLayerIntegrationPlugin::~HardwareLayerIntegrationPlugin()
{
}

}

QT_END_NAMESPACE

#include "moc_qwlhardwarelayerintegrationplugin_p.cpp"
