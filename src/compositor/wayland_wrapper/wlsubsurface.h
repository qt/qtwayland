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

#ifndef WLSUBSURFACE_H
#define WLSUBSURFACE_H

#include "wlsurface.h"

#include "wayland-sub-surface-extension-server-protocol.h"

#include <QtCore/QLinkedList>

class Compositor;
class WaylandSurface;

namespace Wayland {

class SubSurfaceExtensionGlobal
{
public:
    SubSurfaceExtensionGlobal(Compositor *compositor);

private:
    Compositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
    static void get_sub_surface_aware_surface(struct wl_client *client,
                                          struct wl_resource *sub_surface_extension_resource,
                                          uint32_t id,
                                          struct wl_resource *surface_resource);

    static const struct wl_sub_surface_extension_interface sub_surface_extension_interface;
};

class SubSurface
{
public:
    SubSurface(struct wl_client *client, uint32_t id, Surface *surface);
    ~SubSurface();

    void setSubSurface(SubSurface *subSurface, int x, int y);
    void removeSubSurface(SubSurface *subSurfaces);

    SubSurface *parent() const;
    void setParent(SubSurface *parent);

    QLinkedList<WaylandSurface *> subSurfaces() const;

    Surface *surface() const;
    WaylandSurface *waylandSurface() const;

private:
    void parentDestroyed();
    struct wl_resource *m_sub_surface_resource;
    Surface *m_surface;

    SubSurface *m_parent;
    QLinkedList<WaylandSurface *> m_sub_surfaces;

    static void attach_sub_surface(struct wl_client *client,
                                   struct wl_resource *sub_surface_parent_resource,
                                   struct wl_resource *sub_surface_child_resource,
                                   int32_t x,
                                   int32_t y);
    static void move_sub_surface(struct wl_client *client,
                                 struct wl_resource *sub_surface_parent_resource,
                                 struct wl_resource *sub_surface_child_resource,
                                 int32_t x,
                                 int32_t y);
    static void raise(struct wl_client *client,
                      struct wl_resource *sub_surface_parent_resource,
                      struct wl_resource *sub_surface_child_resource);
    static void lower(struct wl_client *client,
                      struct wl_resource *sub_surface_parent_resource,
                      struct wl_resource *sub_surface_child_resource);
    static const struct wl_sub_surface_interface sub_surface_interface;
};

inline Surface *SubSurface::surface() const
{
    return m_surface;
}

inline WaylandSurface *SubSurface::waylandSurface() const
{
    return m_surface->waylandSurface();
}


}

#endif // WLSUBSURFACE_H
