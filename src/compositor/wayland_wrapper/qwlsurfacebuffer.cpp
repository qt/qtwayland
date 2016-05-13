/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlsurfacebuffer_p.h"

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "hardware_integration/qwlclientbufferintegration_p.h"
#include <qpa/qplatformopenglcontext.h>
#endif

#include <QtCore/QDebug>

#include <wayland-server-protocol.h>
#include "qwaylandshmformathelper_p.h"

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

SurfaceBuffer::SurfaceBuffer(QWaylandSurface *surface)
    : m_surface(surface)
    , m_compositor(surface->compositor())
    , m_buffer(0)
    , m_committed(false)
    , m_is_registered_for_buffer(false)
    , m_surface_has_buffer(false)
    , m_destroyed(false)
    , m_is_displayed(false)
    , m_used(false)
    , m_destroyIfUnused(false)
{
}

SurfaceBuffer::~SurfaceBuffer()
{
    if (m_is_registered_for_buffer)
        destructBufferState();
}

void SurfaceBuffer::initialize(struct ::wl_resource *buffer)
{
    m_buffer = buffer;
    m_committed = false;
    m_is_registered_for_buffer = true;
    m_surface_has_buffer = true;
    m_is_displayed = false;
    m_destroyed = false;
    m_destroy_listener.surfaceBuffer = this;
    m_destroy_listener.listener.notify = destroy_listener_callback;
    if (buffer) {
        if (ClientBufferIntegration *integration = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration())
            integration->initializeBuffer(buffer);
        wl_signal_add(&buffer->destroy_signal, &m_destroy_listener.listener);
    }
}

void SurfaceBuffer::destructBufferState()
{
    if (m_buffer) {
        if (m_committed)
            sendRelease();
        wl_list_remove(&m_destroy_listener.listener.link);
    }
    m_buffer = 0;
    m_committed = false;
    m_is_registered_for_buffer = false;
    m_is_displayed = false;
}

void SurfaceBuffer::sendRelease()
{
    Q_ASSERT(m_buffer);
    wl_buffer_send_release(m_buffer);
}

void SurfaceBuffer::disown()
{
    m_surface_has_buffer = false;
    destructBufferState();
    destroyIfUnused();
}

void SurfaceBuffer::setDisplayed()
{
    m_is_displayed = true;
}

void SurfaceBuffer::destroy_listener_callback(wl_listener *listener, void *data)
{
    Q_UNUSED(data);
    struct surface_buffer_destroy_listener *destroy_listener =
        reinterpret_cast<struct surface_buffer_destroy_listener *>(listener);
    SurfaceBuffer *d = destroy_listener->surfaceBuffer;

    // Mark the buffer as destroyed and clear m_buffer right away to avoid
    // touching it before it is properly cleaned up.
    d->m_destroyed = true;
    d->m_buffer = 0;
}

void SurfaceBuffer::ref()
{
    m_used = m_refCount.ref();
}

void SurfaceBuffer::deref()
{
    m_used = m_refCount.deref();
    if (!m_used)
        disown();
}

void SurfaceBuffer::setDestroyIfUnused(bool destroy)
{
    m_destroyIfUnused = destroy;
    destroyIfUnused();
}

void SurfaceBuffer::destroyIfUnused()
{
    if (!m_used && m_destroyIfUnused)
        delete this;
}

QSize SurfaceBuffer::size() const
{
    if (!m_buffer)
        return QSize();

    if (wl_shm_buffer *shmBuffer = wl_shm_buffer_get(m_buffer)) {
        int width = wl_shm_buffer_get_width(shmBuffer);
        int height = wl_shm_buffer_get_height(shmBuffer);
        return QSize(width, height);
    }
    if (ClientBufferIntegration *integration = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration()) {
        return integration->bufferSize(m_buffer);
    }

    return QSize();
}

QWaylandSurface::Origin SurfaceBuffer::origin() const
{
    if (isShm()) {
        return QWaylandSurface::OriginTopLeft;
    }

    if (ClientBufferIntegration *integration = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration()) {
        return integration->origin(m_buffer);
    }
    return QWaylandSurface::OriginTopLeft;
}

QImage SurfaceBuffer::image() const
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

QWaylandBufferRef::BufferFormatEgl SurfaceBuffer::bufferFormatEgl() const
{
    Q_ASSERT(isShm() == false);

    if (QtWayland::ClientBufferIntegration *clientInt = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration())
        return clientInt->bufferFormat(m_buffer);

    return QWaylandBufferRef::BufferFormatEgl_Null;
}

void SurfaceBuffer::bindToTexture() const
{
    Q_ASSERT(m_compositor);
    if (isShm()) {
        QImage image = this->image();
        if (image.hasAlphaChannel()) {
            if (image.format() != QImage::Format_RGBA8888) {
                image = image.convertToFormat(QImage::Format_RGBA8888);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
        } else {
            if (image.format() != QImage::Format_RGBX8888) {
                image = image.convertToFormat(QImage::Format_RGBX8888);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.constBits());
        }
    } else {
        if (QtWayland::ClientBufferIntegration *clientInt = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration()) {
            clientInt->bindTextureToBuffer(m_buffer);
        }
    }
}

uint SurfaceBuffer::textureForPlane(int plane) const
{
    if (QtWayland::ClientBufferIntegration *clientInt = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration())
        return clientInt->textureForBuffer(m_buffer, plane);

    return 0;
}

void SurfaceBuffer::updateTexture() const
{
    if (QtWayland::ClientBufferIntegration *clientInt = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration())
        clientInt->updateTextureForBuffer(m_buffer);
}

}

QT_END_NAMESPACE
