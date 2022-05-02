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

#include "qwaylandhardwareintegration_p.h"

#include "qwaylanddisplay_p.h"
QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandHardwareIntegration::QWaylandHardwareIntegration(struct ::wl_registry *registry, int id)
    : qt_hardware_integration(registry, id, 1)
{
}

QString QWaylandHardwareIntegration::clientBufferIntegration()
{
    return m_client_buffer;
}

QString QWaylandHardwareIntegration::serverBufferIntegration()
{
    return m_server_buffer;
}

void QWaylandHardwareIntegration::hardware_integration_client_backend(const QString &name)
{
    m_client_buffer = name;
}

void QWaylandHardwareIntegration::hardware_integration_server_backend(const QString &name)
{
    m_server_buffer = name;
}

}

QT_END_NAMESPACE
