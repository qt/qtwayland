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

#ifndef SURFACEBUFFER_H
#define SURFACEBUFFER_H

#include <QtCore/QRect>
#include <QtGui/qopengl.h>

#include <wayland-server.h>

class GraphicsHardwareIntegration;
class QOpenGLContext;

namespace Wayland {

struct surface_buffer_destroy_listener
{
    struct wl_listener listener;
    class SurfaceBuffer *surfaceBuffer;
};

class SurfaceBuffer
{
public:
    SurfaceBuffer();


    ~SurfaceBuffer();


    void initialize(struct wl_buffer *buffer);
    void destructBufferState();
    inline bool bufferIsDestroyed() const { return m_is_destroyed; }

    inline int32_t width() const { return m_buffer->width; }
    inline int32_t height() const { return m_buffer->height; }

    inline bool isShmBuffer() const { return wl_buffer_is_shm(m_buffer); }

    inline bool isRegisteredWithBuffer() const { return m_is_registered_for_buffer; }

    void sendRelease();
    void dontSendRelease();

    void setDisplayed();

    inline bool isDisplayed() const { return m_is_displayed; }

    inline QRect damageRect() const { return m_damageRect; }
    void setDamage(const QRect &rect);

    inline bool textureCreated() const { return m_texture; }

    void createTexture(GraphicsHardwareIntegration *hwIntegration, QOpenGLContext *context);
    inline GLuint texture() const;
    void destroyTexture();

    inline struct wl_buffer *handle() const { return m_buffer; }
private:
    struct wl_buffer *m_buffer;
    struct surface_buffer_destroy_listener m_destroy_listener;
    QRect m_damageRect;
    bool m_dont_send_release;
    bool m_is_registered_for_buffer;
    bool m_is_displayed;
    bool m_is_destroyed;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint m_texture;
#else
    uint m_texture;
#endif

    static void destroy_listener_callback(struct wl_listener *listener,
             struct wl_resource *resource, uint32_t time);

};

GLuint SurfaceBuffer::texture() const
{
    if (m_buffer)
        return m_texture;
    return 0;
}

}

#endif // SURFACEBUFFER_H
