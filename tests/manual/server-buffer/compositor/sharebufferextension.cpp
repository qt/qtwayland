// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "sharebufferextension.h"

#include <QWaylandSurface>

#include <QDebug>

#include <QQuickWindow>

#include <QPainter>
#include <QPen>

ShareBufferExtension::ShareBufferExtension(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor)
{
}

void ShareBufferExtension::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);
}

QtWayland::ServerBuffer *ShareBufferExtension::addImage(const QImage &img)
{
    if (!m_server_buffer_integration) {
        QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());

        m_server_buffer_integration = QWaylandCompositorPrivate::get(compositor)->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qWarning("Could not find a server buffer integration");
            return nullptr;
        }
    }

    QImage image = img.convertToFormat(QImage::Format_RGBA8888);

    auto *buffer = m_server_buffer_integration->createServerBufferFromImage(image, QtWayland::ServerBuffer::RGBA32);

    m_server_buffers.append(buffer);
    return buffer;
}

void ShareBufferExtension::createServerBuffers()
{
    QImage image(100,100,QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0x55,0x0,0x55,0x01));
    {
        QPainter p(&image);
        QPen pen = p.pen();
        pen.setWidthF(3);
        pen.setColor(Qt::red);
        p.setPen(pen);
        p.drawLine(0,0,100,100);
        pen.setColor(Qt::green);
        p.setPen(pen);
        p.drawLine(100,0,0,100);
        pen.setColor(Qt::blue);
        p.setPen(pen);
        p.drawLine(25,15,75,15);
    }

    addImage(image);

    QImage image2(":/images/Siberischer_tiger_de_edit02.jpg");
    addImage(image2);

    m_server_buffers_created = true;
}


void ShareBufferExtension::share_buffer_bind_resource(Resource *resource)
{
    if (!m_server_buffers_created)
        createServerBuffers();

    for (auto *buffer : std::as_const(m_server_buffers)) {
        qDebug() << "sending" << buffer << "to client";
        struct ::wl_client *client = wl_resource_get_client(resource->handle);
        struct ::wl_resource *buffer_resource = buffer->resourceForClient(client);
        send_cross_buffer(resource->handle, buffer_resource);
    }
}
