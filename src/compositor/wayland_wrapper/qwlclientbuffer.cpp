/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlclientbuffer_p.h"

#if QT_CONFIG(opengl)
#include "hardware_integration/qwlclientbufferintegration_p.h"
#include <qpa/qplatformopenglcontext.h>
#include <QOpenGLTexture>
#endif

#include <QtCore/QDebug>

#include <wayland-server-protocol.h>
#include "qwaylandsharedmemoryformathelper_p.h"

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

ClientBuffer::ClientBuffer(struct ::wl_resource *buffer)
    : m_buffer(buffer)
    , m_textureDirty(false)
    , m_committed(false)
    , m_destroyed(false)
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

    if (!m_refCount)
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
#if QT_CONFIG(opengl)
    , m_shmTexture(nullptr)
#endif
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


// TODO: support different color formats, and try to avoid QImage::convertToFormat()

QImage SharedMemoryBuffer::image() const
{
    if (wl_shm_buffer *shmBuffer = wl_shm_buffer_get(m_buffer)) {
        int width = wl_shm_buffer_get_width(shmBuffer);
        int height = wl_shm_buffer_get_height(shmBuffer);
        int bytesPerLine = wl_shm_buffer_get_stride(shmBuffer);
        uchar *data = static_cast<uchar *>(wl_shm_buffer_get_data(shmBuffer));
        return QImage(data, width, height, bytesPerLine, QImage::Format_ARGB32_Premultiplied);
    }

    return QImage();
}

#if QT_CONFIG(opengl)
QOpenGLTexture *SharedMemoryBuffer::toOpenGlTexture(int plane)
{
    Q_UNUSED(plane);
    if (isSharedMemory()) {
        if (!m_shmTexture) {
            m_shmTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
            m_shmTexture->create();
        }
        if (m_textureDirty) {
            m_textureDirty = false;
            m_shmTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            // Next 2 settings will handle the case when texture is npot (non power of two)
            // and extension OES_texture_npot is not present
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
        return m_shmTexture;
    }
    return nullptr;
}
#endif

}

QT_END_NAMESPACE
