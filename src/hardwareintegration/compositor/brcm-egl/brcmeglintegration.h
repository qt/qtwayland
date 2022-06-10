// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BRCMEGLINTEGRATION_H
#define BRCMEGLINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include "qwayland-server-brcm.h"

#include <QtCore/QScopedPointer>

#include <private/qwlclientbuffer_p.h>

QT_BEGIN_NAMESPACE

class BrcmEglIntegrationPrivate;

class BrcmEglIntegration : public QtWayland::ClientBufferIntegration, public QtWaylandServer::qt_brcm
{
    Q_DECLARE_PRIVATE(BrcmEglIntegration)
public:
    BrcmEglIntegration();

    void initializeHardware(struct ::wl_display *display) override;
    QtWayland::ClientBuffer *createBufferFor(wl_resource *buffer) override;

protected:
    void brcm_bind_resource(Resource *resource) override;
    void brcm_create_buffer(Resource *resource, uint32_t id, int32_t width, int32_t height, wl_array *data) override;

private:
    Q_DISABLE_COPY(BrcmEglIntegration)
    QScopedPointer<BrcmEglIntegrationPrivate> d_ptr;
};

class BrcmEglClientBuffer : public QtWayland::ClientBuffer
{
public:
    BrcmEglClientBuffer(BrcmEglIntegration *integration, wl_resource *buffer);

    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
private:
    BrcmEglIntegration *m_integration = nullptr;
    QOpenGLTexture *m_texture = nullptr;
};


QT_END_NAMESPACE

#endif // BRCMEGLINTEGRATION_H

