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

#ifndef SURFACEBUFFER_H
#define SURFACEBUFFER_H

#include <QtCore/QRect>
#include <QtGui/qopengl.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <qpa/qplatformscreenpageflipper.h>
#include <QImage>

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

class QWaylandGraphicsHardwareIntegration;
class QOpenGLContext;

namespace QtWayland {

class Surface;
class Compositor;

struct surface_buffer_destroy_listener
{
    struct wl_listener listener;
    class SurfaceBuffer *surfaceBuffer;
};

class SurfaceBuffer : public QPlatformScreenBuffer
{
public:
    SurfaceBuffer(Surface *surface);

    ~SurfaceBuffer();

    void initialize(struct wl_buffer *buffer);
    void destructBufferState();

    inline int32_t width() const { return m_buffer->width; }
    inline int32_t height() const { return m_buffer->height; }

    bool isShmBuffer() const;

    inline bool isRegisteredWithBuffer() const { return m_is_registered_for_buffer; }

    void sendRelease();
    void setPageFlipperHasBuffer(bool owns);
    bool pageFlipperHasBuffer() const { return m_page_flipper_has_buffer; }
    void release();
    void scheduledRelease();
    void disown();

    void setDisplayed();

    inline bool isComitted() const { return m_committed; }
    inline void setCommitted() { m_committed = true; }
    inline bool isDisplayed() const { return m_is_displayed; }

    inline QRect damageRect() const { return m_damageRect; }
    void setDamage(const QRect &rect);

    inline bool textureCreated() const { return m_texture; }

    void createTexture(QWaylandGraphicsHardwareIntegration *hwIntegration, QOpenGLContext *context);
    inline GLuint texture() const;
    void destroyTexture();

    inline struct wl_buffer *waylandBufferHandle() const { return m_buffer; }

    void handleAboutToBeDisplayed();
    void handleDisplayed();

    void *handle() const;
    QImage image();
private:
    Surface *m_surface;
    Compositor *m_compositor;
    struct wl_buffer *m_buffer;
    struct surface_buffer_destroy_listener m_destroy_listener;
    QRect m_damageRect;
    bool m_committed;
    bool m_is_registered_for_buffer;
    bool m_surface_has_buffer;
    bool m_page_flipper_has_buffer;

    bool m_is_displayed;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint m_texture;
    QOpenGLSharedResourceGuard *m_guard;
#else
    uint m_texture;
    uint m_guard;
#endif
    void *m_handle;
    bool m_is_shm_resolved;
    bool m_is_shm;

    QImage m_image;

    static void destroy_listener_callback(wl_listener *listener, void *data);
};

GLuint SurfaceBuffer::texture() const
{
    if (m_buffer)
        return m_texture;
    return 0;
}

}

QT_END_NAMESPACE

#endif // SURFACEBUFFER_H
