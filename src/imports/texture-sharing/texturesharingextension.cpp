// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "texturesharingextension_p.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QWindow>
#include <QtGui/QPlatformSurfaceEvent>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

TextureSharingExtension::TextureSharingExtension()
    : QWaylandClientExtensionTemplate(/* Supported protocol version */ 1 )
{
        auto *wayland_integration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
        m_server_buffer_integration = wayland_integration->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qCritical() << "This application requires a working serverBufferIntegration";
            QGuiApplication::quit();
        }
}

void TextureSharingExtension::zqt_texture_sharing_v1_provide_buffer(struct ::qt_server_buffer *buffer, const QString &key)
{
    QtWaylandClient::QWaylandServerBuffer *serverBuffer = m_server_buffer_integration->serverBuffer(buffer);
    emit bufferReceived(serverBuffer, key);
}

void TextureSharingExtension::zqt_texture_sharing_v1_image_failed(const QString &key, const QString &message)
{
    qWarning() << "TextureSharingExtension" << key << "not found" << message;
    emit bufferReceived(nullptr, key);
}
void TextureSharingExtension::requestImage(const QString &key)
{
    request_image(key);
}

void TextureSharingExtension::abandonImage(const QString &key)
{
    abandon_image(key);
}

QT_END_NAMESPACE

#include "moc_texturesharingextension_p.cpp"
