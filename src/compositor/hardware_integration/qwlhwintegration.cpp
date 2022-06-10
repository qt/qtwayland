// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlhwintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>

QT_BEGIN_NAMESPACE

namespace QtWayland {

HardwareIntegration::HardwareIntegration(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<HardwareIntegration>(compositor)
    , qt_hardware_integration(compositor->display(), 1)
{
}

void HardwareIntegration::setClientBufferIntegrationName(const QString &name)
{
    m_client_buffer_integration_name = name;
}
void HardwareIntegration::setServerBufferIntegrationName(const QString &name)
{
    m_server_buffer_integration_name = name;
}

void HardwareIntegration::hardware_integration_bind_resource(Resource *resource)
{
    if (!m_client_buffer_integration_name.isEmpty())
        send_client_backend(resource->handle, m_client_buffer_integration_name);
    if (!m_server_buffer_integration_name.isEmpty())
        send_server_backend(resource->handle, m_server_buffer_integration_name);
}

}

QT_END_NAMESPACE
