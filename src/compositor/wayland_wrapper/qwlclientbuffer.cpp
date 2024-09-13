// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlclientbuffer_p.h"

#if QT_CONFIG(opengl)
#include "hardware_integration/qwlclientbufferintegration_p.h"
#include <qpa/qplatformopenglcontext.h>
#include <QOpenGLTexture>
#endif

#include <QtCore/QDebug>

#include <QtWaylandCompositor/private/wayland-wayland-server-protocol.h>
#include "qwaylandsharedmemoryformathelper_p.h"

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

ClientBuffer::ClientBuffer(struct ::wl_resource *buffer)
    : m_buffer(buffer)
{
}


ClientBuffer::~ClientBuffer()
{
    if (m_buffer && m_committed && !m_destroyed)
        sendRelease();
}

void ClientBuffer::sendRelease()
{
    Q_ASSERT(m_buffer);
    wl_buffer_send_release(m_buffer);
    m_committed = false;
}

void ClientBuffer::setDestroyed()
{
    m_destroyed = true;
    m_committed = false;
    m_buffer = nullptr;

    if (!m_refCount.loadAcquire())
        delete this;
}

void ClientBuffer::ref()
{
    m_refCount.ref();
}

void ClientBuffer::deref()
{
    if (!m_refCount.deref()) {
        if (isCommitted() && m_buffer && !m_destroyed)
            sendRelease();
        if (m_destroyed)
            delete this;
    }
}

void ClientBuffer::setCommitted(QRegion &damage)
{
     m_damage = damage;
     m_committed = true;
     m_textureDirty = true;
}

QWaylandBufferRef::BufferFormatEgl ClientBuffer::bufferFormatEgl() const
{
    return QWaylandBufferRef::BufferFormatEgl_Null;
}

SharedMemoryBuffer::SharedMemoryBuffer(wl_resource *bufferResource)
    : ClientBuffer(bufferResource)
{

}

QSize SharedMemoryBuffer::size() const
{
    if (wl_shm_buffer *shmBuffer = wl_shm_buffer_get(m_buffer)) {
        int width = wl_shm_buffer_get_width(shmBuffer);
        int height = wl_shm_buffer_get_height(shmBuffer);
        return QSize(width, height);
    }
    return QSize();
}

QWaylandSurface::Origin SharedMemoryBuffer::origin() const
{
    return QWaylandSurface::OriginTopLeft;
}

static void shmBufferCleanup(void *data)
{
    auto *pool = static_cast<struct wl_shm_pool *>(data);
    wl_shm_pool_unref(pool);
}

QImage SharedMemoryBuffer::image() const
{
    if (wl_shm_buffer *shmBuffer = wl_shm_buffer_get(m_buffer)) {
        int width = wl_shm_buffer_get_width(shmBuffer);
        int height = wl_shm_buffer_get_height(shmBuffer);
        int bytesPerLine = wl_shm_buffer_get_stride(shmBuffer);

        // TODO: try to avoid QImage::convertToFormat()
        wl_shm_format shmFormat = wl_shm_format(wl_shm_buffer_get_format(shmBuffer));
        QImage::Format format = QWaylandSharedMemoryFormatHelper::fromWaylandShmFormat(shmFormat);

        auto *pool = wl_shm_buffer_ref_pool(shmBuffer);
        uchar *data = static_cast<uchar *>(wl_shm_buffer_get_data(shmBuffer));
        return QImage(data, width, height, bytesPerLine, format, &shmBufferCleanup, pool);
    }

    return QImage();
}

#if QT_CONFIG(opengl)
QOpenGLTexture *SharedMemoryBuffer::toOpenGlTexture(int plane)
{
    Q_UNUSED(plane);
    if (isSharedMemory()) {
        if (!m_shmTexture) {
            m_shmTexture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
            m_shmTexture->create();
        }
        if (m_textureDirty) {
            m_textureDirty = false;
            m_shmTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            // TODO: partial texture upload
            QImage image = this->image();
            m_shmTexture->setSize(image.width(), image.height());
            if (image.hasAlphaChannel()) {
                m_shmTexture->setFormat(QOpenGLTexture::RGBAFormat);
                if (image.format() != QImage::Format_RGBA8888)
                    image = image.convertToFormat(QImage::Format_RGBA8888);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
            } else {
                m_shmTexture->setFormat(QOpenGLTexture::RGBFormat);
                if (image.format() != QImage::Format_RGBX8888)
                    image = image.convertToFormat(QImage::Format_RGBX8888);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.constBits());
            }
            //we can release the buffer after uploading, since we have a copy
            if (isCommitted())
                sendRelease();
        }
        return m_shmTexture.data();
    }
    return nullptr;
}
#endif

}

QT_END_NAMESPACE
