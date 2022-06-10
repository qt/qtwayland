// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHMSERVERBUFFERINTEGRATION_H
#define SHMSERVERBUFFERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-shm-emulation-server-buffer.h"

#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwayland-server-server-buffer-extension.h>

QT_BEGIN_NAMESPACE

class ShmServerBufferIntegration;
class QSharedMemory;

class ShmServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    ShmServerBuffer(ShmServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);
    ~ShmServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;

private:
    ShmServerBufferIntegration *m_integration = nullptr;

    QSharedMemory *m_shm = nullptr;
    int m_width;
    int m_height;
    int m_bpl;
    QOpenGLTexture *m_texture = nullptr;
    QtWaylandServer::qt_shm_emulation_server_buffer::format m_shm_format;
};

class ShmServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_shm_emulation_server_buffer
{
public:
    ShmServerBufferIntegration();
    ~ShmServerBufferIntegration() override;

    bool initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;


private:
};

QT_END_NAMESPACE

#endif
