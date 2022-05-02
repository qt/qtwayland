/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

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
