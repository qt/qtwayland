/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef QWAYLANDEGLPLATFORMINTEGRATION_H
#define QWAYLANDEGLPLATFORMINTEGRATION_H

#include <QtWaylandClient/private/qwaylandintegration_p.h>

#include "qwaylandeglclientbufferintegration.h"

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
