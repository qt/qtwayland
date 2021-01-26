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

#ifndef WAYLANDEGLINTEGRATION_H
#define WAYLANDEGLINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QtCore/QScopedPointer>
#include <QtWaylandCompositor/private/qwlclientbuffer_p.h>

QT_BEGIN_NAMESPACE

class WaylandEglClientBufferIntegrationPrivate;

class WaylandEglClientBufferIntegration : public QtWayland::ClientBufferIntegration
{
    Q_DECLARE_PRIVATE(WaylandEglClientBufferIntegration)
public:
    WaylandEglClientBufferIntegration();
    ~WaylandEglClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;

    QtWayland::ClientBuffer *createBufferFor(wl_resource *buffer) override;

private:
    Q_DISABLE_COPY(WaylandEglClientBufferIntegration)
    QScopedPointer<WaylandEglClientBufferIntegrationPrivate> d_ptr;
};

struct BufferState;

class WaylandEglClientBuffer : public QtWayland::ClientBuffer
{
public:
    ~WaylandEglClientBuffer() override;

    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    quintptr lockNativeBuffer() override;
    void unlockNativeBuffer(quintptr native_buffer) const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    void setCommitted(QRegion &damage) override;

private:
    friend class WaylandEglClientBufferIntegration;
    friend class WaylandEglClientBufferIntegrationPrivate;

    WaylandEglClientBuffer(WaylandEglClientBufferIntegration* integration, wl_resource *bufferResource);

    BufferState *d = nullptr;
    WaylandEglClientBufferIntegration *m_integration = nullptr;
};

QT_END_NAMESPACE

#endif // WAYLANDEGLINTEGRATION_H
