/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
****************************************************************************/

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

QT_BEGIN_NAMESPACE

class QWaylandCompositor;

namespace QtWayland {

class HardwareIntegration : public QWaylandCompositorExtensionTemplate<HardwareIntegration>, public QtWaylandServer::qt_hardware_integration
{
public:
    HardwareIntegration(QWaylandCompositor *compositor);

    void setClientBufferIntegration(const QString &name);
    void setServerBufferIntegration(const QString &name);

protected:
    void hardware_integration_bind_resource(Resource *resource) override;

private:
    QString m_client_buffer_integration;
    QString m_server_buffer_integration;
};

}

QT_END_NAMESPACE
#endif
