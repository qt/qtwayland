// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

#ifndef WAYLANDEGLINTEGRATION_H
#define WAYLANDEGLINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QtCore/QScopedPointer>
#include <QtWaylandCompositor/private/qwlclientbuffer_p.h>

QT_BEGIN_NAMESPACE

class WaylandEglClientBufferIntegrationPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandEglClientBufferIntegration : public QtWayland::ClientBufferIntegration
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

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandEglClientBuffer : public QtWayland::ClientBuffer
{
public:
    WaylandEglClientBuffer(WaylandEglClientBufferIntegration* integration, wl_resource *bufferResource);
    ~WaylandEglClientBuffer() override;

    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    quintptr lockNativeBuffer() override;
    void unlockNativeBuffer(quintptr native_buffer) const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    void setCommitted(QRegion &damage) override;
    bool isProtected() override;

private:
    friend class WaylandEglClientBufferIntegration;
    friend class WaylandEglClientBufferIntegrationPrivate;

    BufferState *d = nullptr;
    WaylandEglClientBufferIntegration *m_integration = nullptr;
};

QT_END_NAMESPACE

#endif // WAYLANDEGLINTEGRATION_H
