// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SHAREBUFFEREXTENSION_H
#define SHAREBUFFEREXTENSION_H

#include "wayland-util.h"

#include <QtCore/QMap>

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-share-buffer.h"

QT_BEGIN_NAMESPACE

namespace QtWayland
{
    class ServerBufferIntegration;
};


class ShareBufferExtension : public QWaylandCompositorExtensionTemplate<ShareBufferExtension>
        , public QtWaylandServer::qt_share_buffer
{
    Q_OBJECT
public:
    ShareBufferExtension(QWaylandCompositor *compositor = nullptr);
    void initialize() override;

protected slots:
    QtWayland::ServerBuffer *addImage(const QImage &image);

protected:
    void share_buffer_bind_resource(Resource *resource) override;

private:
    void createServerBuffers();
    QList<QtWayland::ServerBuffer *> m_server_buffers;
    QtWayland::ServerBufferIntegration *m_server_buffer_integration = nullptr;
    bool m_server_buffers_created = false;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(ShareBufferExtension)

QT_END_NAMESPACE

#endif // SHAREBUFFEREXTENSION_H
