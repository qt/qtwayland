/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlsurfacebuffer_p.h"

#include "qwlsurface_p.h"
#include "qwlcompositor_p.h"

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include "hardware_integration/qwaylandgraphicshardwareintegration.h"
#include <qpa/qplatformopenglcontext.h>
#endif

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

SurfaceBuffer::SurfaceBuffer(Surface *surface)
    : QPlatformScreenBuffer()
    , m_surface(surface)
    , m_compositor(surface->compositor())
    , m_buffer(0)
    , m_is_registered_for_buffer(false)
    , m_surface_has_buffer(false)
    , m_page_flipper_has_buffer(false)
    , m_is_displayed(false)
    , m_texture(0)
    , m_guard(0)
    , m_is_shm_resolved(false)
    , m_is_shm(false)
{
}

SurfaceBuffer::~SurfaceBuffer()
{
    if (m_is_registered_for_buffer)
        destructBufferState();
}

void SurfaceBuffer::initialize(wl_buffer *buffer)
{
    m_buffer = buffer;
    m_texture = 0;
    m_guard = 0;
    m_is_registered_for_buffer = true;
    m_surface_has_buffer = true;
    m_page_flipper_has_buffer = false;
    m_is_displayed = false;
    m_destroyed = false;
    m_handle = 0;
    m_is_shm_resolved = false;
    m_is_shm = false;
    m_destroy_listener.surfaceBuffer = this;
    m_destroy_listener.listener.notify = destroy_listener_callback;
    if (buffer)
        wl_signal_add(&buffer->resource.destroy_signal, &m_destroy_listener.listener);
    m_damageRect = QRect();
}

void SurfaceBuffer::destructBufferState()
{
    Q_ASSERT(!m_page_flipper_has_buffer);

    destroyTexture();

    if (m_buffer) {
        if (m_handle) {
            if (m_is_shm) {
                delete static_cast<QImage *>(m_handle);
#ifdef QT_COMPOSITOR_WAYLAND_GL
            } else {
                QWaylandGraphicsHardwareIntegration *hwIntegration = m_compositor->graphicsHWIntegration();
                hwIntegration->unlockNativeBuffer(m_handle, m_compositor->directRenderContext());
#endif
            }
        }
        wl_list_remove(&m_destroy_listener.listener.link);
        sendRelease();
    }
    m_buffer = 0;
    m_handle = 0;
    m_is_registered_for_buffer = false;
    m_is_displayed = false;
}

bool SurfaceBuffer::isShmBuffer() const
{
    if (!m_is_shm_resolved) {
        SurfaceBuffer *that = const_cast<SurfaceBuffer *>(this);
        that->m_is_shm = wl_buffer_is_shm(m_buffer);
        that->m_is_shm_resolved = true;
    }
    return m_is_shm;
}

void SurfaceBuffer::sendRelease()
{
    Q_ASSERT(m_buffer);
    wl_buffer_send_release(&m_buffer->resource);
}

void SurfaceBuffer::setPageFlipperHasBuffer(bool owns)
{
    m_page_flipper_has_buffer = owns;
}

void SurfaceBuffer::release()
{
    m_compositor->scheduleReleaseBuffer(this);
}

void SurfaceBuffer::scheduledRelease()
{
    m_page_flipper_has_buffer = false;
    if (!m_surface_has_buffer)
        destructBufferState();
    if (!m_surface) {
        delete this;
    }
}

void SurfaceBuffer::disown()
{
    m_surface_has_buffer = false;

    if (!m_page_flipper_has_buffer) {
        destructBufferState();
    }
}

void SurfaceBuffer::setDisplayed()
{
    m_is_displayed = true;
    m_damageRect = QRect();
}

void SurfaceBuffer::setDamage(const QRect &rect)
{
        if (m_damageRect.isValid()) {
        m_damageRect = m_damageRect.united(rect);
    }
    m_damageRect = rect;
}

void SurfaceBuffer::destroyTexture()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_texture) {
        Q_ASSERT(m_guard);
        m_guard->free();
        m_guard = 0;
        m_texture = 0;
    }
#endif
}

void SurfaceBuffer::handleAboutToBeDisplayed()
{
    qDebug() << Q_FUNC_INFO;
}

void SurfaceBuffer::handleDisplayed()
{
    qDebug() << Q_FUNC_INFO;
}

void *SurfaceBuffer::handle() const
{
    if (!m_buffer)
        return 0;

    if (!m_handle) {
        SurfaceBuffer *that = const_cast<SurfaceBuffer *>(this);
        if (isShmBuffer()) {
            const uchar *data = static_cast<const uchar *>(wl_shm_buffer_get_data(m_buffer));
            int stride = wl_shm_buffer_get_stride(m_buffer);
            int width = m_buffer->width;
            int height = m_buffer->height;
            QImage *image = new QImage(data,width,height,stride, QImage::Format_ARGB32_Premultiplied);
            that->m_handle = image;
#ifdef QT_COMPOSITOR_WAYLAND_GL
        } else {
            QWaylandGraphicsHardwareIntegration *hwIntegration = m_compositor->graphicsHWIntegration();
            that->m_handle = hwIntegration->lockNativeBuffer(m_buffer, m_compositor->directRenderContext());
#endif
        }
    }
    return m_handle;
}

void SurfaceBuffer::destroy_listener_callback(wl_listener *listener, void *data)
{
        Q_UNUSED(data);
        struct surface_buffer_destroy_listener *destroy_listener =
                reinterpret_cast<struct surface_buffer_destroy_listener *>(listener);
        SurfaceBuffer *d = destroy_listener->surfaceBuffer;
        d->destroyTexture();
        d->m_destroyed = true;
        d->m_buffer = 0;
}

void freeTexture(QOpenGLFunctions *, GLuint id)
{
    glDeleteTextures(1, &id);
}

void SurfaceBuffer::createTexture(QWaylandGraphicsHardwareIntegration *hwIntegration, QOpenGLContext *context)
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    m_texture = hwIntegration->createTextureFromBuffer(m_buffer, context);
    m_guard = new QOpenGLSharedResourceGuard(QOpenGLContext::currentContext(), m_texture, freeTexture);
#else
    Q_UNUSED(hwIntegration);
    Q_UNUSED(context);
#endif
}

}

QT_END_NAMESPACE
