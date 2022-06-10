// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLHWINTEGRATION_P_H
#define QWLHWINTEGRATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/private/qwayland-server-hardware-integration.h>

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QString>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;

namespace QtWayland {

class HardwareIntegration : public QWaylandCompositorExtensionTemplate<HardwareIntegration>, public QtWaylandServer::qt_hardware_integration
{
public:
    HardwareIntegration(QWaylandCompositor *compositor);

    void setClientBufferIntegrationName(const QString &name);
    void setServerBufferIntegrationName(const QString &name);

protected:
    void hardware_integration_bind_resource(Resource *resource) override;

private:
    QString m_client_buffer_integration_name;
    QString m_server_buffer_integration_name;
};

}

QT_END_NAMESPACE
#endif
