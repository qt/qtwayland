// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDEGLPLATFORMINTEGRATION_H
#define QWAYLANDEGLPLATFORMINTEGRATION_H

#include <QtWaylandClient/private/qwaylandintegration_p.h>

#include <QtWaylandEglClientHwIntegration/private/qwaylandeglclientbufferintegration_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandEglPlatformIntegration : public QWaylandIntegration
{
public:
    QWaylandEglPlatformIntegration()
        : m_client_buffer_integration(new QWaylandEglClientBufferIntegration())
    {
        m_client_buffer_integration->initialize(display());
    }

    QWaylandEglClientBufferIntegration *clientBufferIntegration() const override
    { return m_client_buffer_integration; }

private:
    QWaylandEglClientBufferIntegration *m_client_buffer_integration;
};

}

QT_END_NAMESPACE

#endif
