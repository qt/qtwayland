/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WLEXTENDEDSURFACE_H
#define WLEXTENDEDSURFACE_H

#include "wayland-surface-extension-server-protocol.h"

#include "wlsurface.h"

#include <QtCore/QVariant>
#include <QtCore/QLinkedList>

class WaylandSurface;

namespace Wayland {

class Compositor;

class SurfaceExtensionGlobal
{
public:
    SurfaceExtensionGlobal(Compositor *compositor);

private:
    Compositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
    static void get_extended_surface(struct wl_client *client,
                                 struct wl_resource *resource,
                                 uint32_t id,
                                 struct wl_resource *surface);
    static const struct wl_surface_extension_interface surface_extension_interface;

};

class ExtendedSurface
{
public:
    ExtendedSurface(struct wl_client *client, uint32_t id, Surface *surface);
    ~ExtendedSurface();

    void sendGenericProperty(const char *name, const QVariant &variant);
    void sendOnScreenVisibllity(bool visible);

    void setSubSurface(ExtendedSurface *subSurface,int x, int y);
    void removeSubSurface(ExtendedSurface *subSurfaces);
    ExtendedSurface *parent() const;
    void setParent(ExtendedSurface *parent);
    QLinkedList<WaylandSurface *> subSurfaces() const;

private:
    struct wl_resource *m_extended_surface_resource;
    Surface *m_surface;

    static void update_generic_property(struct wl_client *client,
                                    struct wl_resource *resource,
                                    const char *name,
                                    struct wl_array *value);
    static void map_sub_surface(struct wl_client *client,
                            struct wl_resource *extended_surface_resource,
                            struct wl_resource *sub_surface_resource,
                            int32_t x,
                            int32_t y);
    static void move_sub_surface(struct wl_client *client,
                             struct wl_resource *extended_surface_resource,
                             struct wl_resource *sub_surface_resource,
                             int32_t x,
                             int32_t y);

    static const struct wl_extended_surface_interface extended_surface_interface;
};

}

#endif // WLEXTENDEDSURFACE_H
