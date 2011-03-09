/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "wlsurface.h"

#include "wlcompositor.h"
#include "wlshmbuffer.h"

#include <QtCore/QDebug>

#include <wayland-server.h>

#ifdef QT_COMPOSITOR_WAYLAND_EGL
#include <QtGui/QPlatformNativeInterface>
#include <QtGui/QPlatformGLContext>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

namespace Wayland {

class SurfacePrivate
{
public:
    SurfacePrivate(struct wl_client *client, Compositor *compositor)
        : m_client(client)
        , m_compositor(compositor)
    {
        current.type = State::Invalid;
        staged.type = State::Invalid;
#ifdef QT_COMPOSITOR_WAYLAND_EGL
        if (QWidget *topLevel = m_compositor->topLevelWidget()) {
            if (topLevel->platformWindow() && topLevel->platformWindow()->glContext()) {
                topLevel->platformWindow()->glContext()->makeCurrent();
                glGenTextures(1,&current.texture_id);
                staged.texture_id = current.texture_id;
            }
        }
#endif
    }

    struct State {
        enum BufferType {
            Shm,
            Texture,
            Invalid
        };

        BufferType type;

        ShmBuffer *shm_buffer;
#ifdef QT_COMPOSITOR_WAYLAND_EGL
        GLuint texture_id;
#endif
        QRect rect;

    };

    State current;
    State staged;

    struct wl_client *m_client;
    Compositor *m_compositor;
};



void surface_destroy(struct wl_client *client, struct wl_surface *surface)
{
    wl_resource_destroy(&surface->resource, client);
}

void surface_attach(struct wl_client *client, struct wl_surface *surface,
                    struct wl_buffer *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);

    if (buffer->attach) {
        wayland_cast<Surface *>(surface)->attachShm(wayland_cast<ShmBuffer *>(buffer));
    }
#ifdef QT_COMPOSITOR_WAYLAND_EGL
    else {
        wayland_cast<Surface *>(surface)->attachEgl(buffer);
    }
#endif //QT_COMPOSITOR_WAYLAND_EGL
}

void surface_map_toplevel(struct wl_client *client,
                          struct wl_surface *surface)
{
    Q_UNUSED(client);
    printf("surface_map_toplevel: %p, %p", client, surface);

    wayland_cast<Surface *>(surface)->mapTopLevel();
}

void surface_map_transient(struct wl_client *client,
                      struct wl_surface *surface,
                      struct wl_surface *parent,
                      int x,
                      int y,
                      uint32_t flags)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(parent);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(flags);
    printf("surface_map_transient: %p, %p", client, surface);
}

void surface_map_fullscreen(struct wl_client *client,
                       struct wl_surface *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    printf("surface_map_fullscreen: %p, %p", client, surface);
}

void surface_damage(struct wl_client *client, struct wl_surface *surface,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    wayland_cast<Surface *>(surface)->damage(QRect(x, y, width, height));
}

Surface::Surface(struct wl_client *client, Compositor *compositor)
    : d_ptr(new SurfacePrivate(client,compositor))
{
    base()->client = client;
    wl_list_init(&base()->destroy_listener_list);
}

Surface::~Surface()
{
    Q_D(Surface);
    d->m_compositor->surfaceDestroyed(this);
}
void Surface::damage(const QRect &rect)
{
    Q_D(Surface);
    d->m_compositor->surfaceDamaged(this, rect);
}

bool Surface::hasImage() const
{
    Q_D(const Surface);
    return d->current.type == SurfacePrivate::State::Shm;
}

QImage Surface::image() const
{
    Q_D(const Surface);
    if (d->current.type == SurfacePrivate::State::Shm) {
        return d->current.shm_buffer->image();
    }
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_EGL
void Surface::attachEgl(wl_buffer *egl_buffer)
{
    Q_D(Surface);
    Q_ASSERT(d->m_compositor->topLevelWidget());
    QPlatformNativeInterface *nativeInterface = QApplicationPrivate::platformIntegration()->nativeInterface();
    Q_ASSERT(nativeInterface);

    //make current for the topLevel. We could have used the eglContext,
    //but then we would need to handle eglSurfaces as well.
    d->m_compositor->topLevelWidget()->platformWindow()->glContext()->makeCurrent();

    EGLDisplay eglDisplay = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWidget("EglDisplay",d->m_compositor->topLevelWidget()));
    EGLContext eglContext = static_cast<EGLContext>(nativeInterface->nativeResourceForWidget("EglContext",d->m_compositor->topLevelWidget()));
    Q_ASSERT(eglDisplay);
    Q_ASSERT(eglContext);

    EGLImageKHR image = eglCreateImageKHR(eglDisplay, eglContext,
                               EGL_WAYLAND_BUFFER_WL,
                               egl_buffer, NULL);

     glBindTexture(GL_TEXTURE_2D, d->staged.texture_id);

     glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

     eglDestroyImageKHR(eglDisplay, image);

     d->staged.type = SurfacePrivate::State::Texture;
     d->current.type = d->staged.type;
     d->current.texture_id = d->staged.texture_id;

     d->m_compositor->surfaceResized(this,QSize(egl_buffer->width,egl_buffer->height));
}

bool Surface::hasTexture() const
{
    Q_D(const Surface);
    return (d->current.type == SurfacePrivate::State::Texture);
}

GLuint Surface::textureId() const
{
    Q_D(const Surface);
    return d->current.texture_id;
}
#endif // QT_COMPOSITOR_WAYLAND_EGL

void Surface::attachShm(Wayland::ShmBuffer *shm_buffer)
{
    Q_D(Surface);
    d->current.shm_buffer = d->current.shm_buffer = shm_buffer;
    d->current.type = d->staged.type = SurfacePrivate::State::Shm;
    d->m_compositor->surfaceResized(this, shm_buffer->size());
}


void Surface::mapTopLevel()
{
    Q_D(Surface);
    d->staged.rect = QRect(0, 0, 200, 200);
}

void Surface::commit()
{
    Q_D(Surface);
    d->current = d->staged;
}

}
