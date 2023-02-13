// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "sharebufferextension.h"
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

ShareBufferExtension::ShareBufferExtension()
    : QWaylandClientExtensionTemplate(/* Supported protocol version */ 1 )
{

        auto *wayland_integration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
        m_server_buffer_integration = wayland_integration->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qCritical() << "This application requires a working serverBufferIntegration";
            QGuiApplication::quit();
        }
}

void ShareBufferExtension::share_buffer_cross_buffer(struct ::qt_server_buffer *buffer)
{
    QtWaylandClient::QWaylandServerBuffer *serverBuffer = m_server_buffer_integration->serverBuffer(buffer);
    emit bufferReceived(serverBuffer);
}


QT_END_NAMESPACE
