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

#ifndef WL_SURFACE_H
#define WL_SURFACE_H

#include "waylandobject.h"
#include "wlshmbuffer.h"
#include "waylandsurface.h"

#include <QtCore/QRect>
#include <QtGui/QImage>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>
#include <QtGui/private/qapplication_p.h>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtOpenGL/QGLContext>
#endif

namespace Wayland {

class Compositor;
class Buffer;

class SurfacePrivate;

class Surface : public Object<struct wl_surface>
{
    Q_DECLARE_PRIVATE(Surface)
public:
    Surface(struct wl_client *client, Compositor *compositor);
    ~Surface();

    WaylandSurface::Type type() const;

    uint id() const { return base()->resource.object.id; }
    void attach(struct wl_buffer *buffer);

    void mapTopLevel();

    void damage(const QRect &rect);

    QImage image() const;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint textureId() const;
#endif

    void sendMousePressEvent(int x, int y, Qt::MouseButton button);
    void sendMouseReleaseEvent(int x, int y, Qt::MouseButton button);
    void sendMouseMoveEvent(int x, int y);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void setInputFocus();

    WaylandSurface *handle() const;
protected:
    QScopedPointer<SurfacePrivate> d_ptr;
private:
    Q_DISABLE_COPY(Surface)
};

void surface_destroy(struct wl_client *client, struct wl_surface *_surface);
void surface_attach(struct wl_client *client, struct wl_surface *surface,
                    struct wl_buffer *buffer, int x, int y);
void surface_map_toplevel(struct wl_client *client,
                          struct wl_surface *surface);
void surface_map_transient(struct wl_client *client,
                      struct wl_surface *surface,
                      struct wl_surface *parent,
                      int x,
                      int y,
                      uint32_t flags);
void surface_map_fullscreen(struct wl_client *client,
                       struct wl_surface *surface);
void surface_damage(struct wl_client *client, struct wl_surface *_surface,
               int32_t x, int32_t y, int32_t width, int32_t height);

const static struct wl_surface_interface surface_interface = {
    surface_destroy,
    surface_attach,
    surface_map_toplevel,
    surface_map_transient,
    surface_map_fullscreen,
    surface_damage
};
}

#endif //WL_SURFACE_H
