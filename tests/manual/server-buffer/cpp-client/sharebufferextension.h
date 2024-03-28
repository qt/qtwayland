// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHAREBUFFEREXTENSION_H
#define SHAREBUFFEREXTENSION_H

#include <qpa/qwindowsysteminterface.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qwaylandclientextension.h>
#include "qwayland-share-buffer.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {
    class QWaylandServerBuffer;
    class QWaylandServerBufferIntegration;
};

class ShareBufferExtension : public QWaylandClientExtensionTemplate<ShareBufferExtension>
        , public QtWayland::qt_share_buffer
{
    Q_OBJECT
public:
    ShareBufferExtension();

signals:
    void bufferReceived(QtWaylandClient::QWaylandServerBuffer *buffer);

private:
    void share_buffer_cross_buffer(struct ::qt_server_buffer *buffer) override;
    QtWaylandClient::QWaylandServerBufferIntegration *m_server_buffer_integration = nullptr;
};

QT_END_NAMESPACE

#endif // SHAREBUFFEREXTENSION_H
