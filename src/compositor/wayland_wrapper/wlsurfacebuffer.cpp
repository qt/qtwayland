/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#include "wlsurfacebuffer.h"

namespace Wayland {

SurfaceBuffer::SurfaceBuffer()
    : m_buffer(0)
    , m_is_released_sent(false)
    , m_is_registered_for_buffer(false)
    , m_is_posted(false)
    , m_is_frame_finished(false)
    , m_texture(0)
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
    m_is_released_sent = false;
    m_is_registered_for_buffer = true;
    m_is_posted = false;
    m_is_frame_finished = false;
    m_destroy_listener.surfaceBuffer = this;
    m_destroy_listener.listener.func = destroy_listener_callback;
    wl_list_insert(&buffer->resource.destroy_listener_list,&m_destroy_listener.listener.link);
    m_damageRect = QRect();
}

void SurfaceBuffer::destructBufferState()
{
    destroyTexture();
    if (m_buffer) {
        wl_list_remove(&m_destroy_listener.listener.link);
        sendRelease();
    }
    m_buffer = 0;
    m_is_registered_for_buffer = false;
    m_is_posted = 0;
}

void SurfaceBuffer::sendRelease()
{
    Q_ASSERT(m_buffer);
    wl_resource_post_event(&m_buffer->resource, WL_BUFFER_RELEASE);
    m_buffer = 0;
    m_is_released_sent = true;
}

void SurfaceBuffer::setPosted()
{
    m_is_posted = true;
     if (m_buffer) {
        wl_list_remove(&m_destroy_listener.listener.link);
     }
     m_buffer = 0;
}

void SurfaceBuffer::setFinished()
{
    m_is_frame_finished = true;
}

void SurfaceBuffer::destroyTexture()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
        if (m_texture) {
            glDeleteTextures(1,&m_texture);
            m_texture = 0;
        }
#endif
}

void SurfaceBuffer::destroy_listener_callback(wl_listener *listener, wl_resource *resource, uint32_t time)
{
        Q_UNUSED(resource);
        Q_UNUSED(time);
        struct surface_buffer_destroy_listener *destroy_listener =
                reinterpret_cast<struct surface_buffer_destroy_listener *>(listener);
        SurfaceBuffer *d = destroy_listener->surfaceBuffer;
        d->destroyTexture();
        d->m_buffer = 0;
}

void SurfaceBuffer::setTexture(GLuint texId)
{
    m_texture = texId;
}

}
