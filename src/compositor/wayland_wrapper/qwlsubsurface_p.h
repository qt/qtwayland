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

#ifndef WLSUBSURFACE_H
#define WLSUBSURFACE_H

#include <private/qwlsurface_p.h>

#include <QtCompositor/private/wayland-sub-surface-extension-server-protocol.h>

#include <QtCore/QLinkedList>

QT_BEGIN_NAMESPACE

class Compositor;
class QWaylandSurface;

namespace QtWayland {

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

    static const struct qt_sub_surface_extension_interface sub_surface_extension_interface;
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

    QLinkedList<QWaylandSurface *> subSurfaces() const;

    Surface *surface() const;
    QWaylandSurface *waylandSurface() const;

private:
    void parentDestroyed();
    struct wl_resource *m_sub_surface_resource;
    Surface *m_surface;

    SubSurface *m_parent;
    QLinkedList<QWaylandSurface *> m_sub_surfaces;

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
    static const struct qt_sub_surface_interface sub_surface_interface;
};

inline Surface *SubSurface::surface() const
{
    return m_surface;
}

inline QWaylandSurface *SubSurface::waylandSurface() const
{
    return m_surface->waylandSurface();
}

}

QT_END_NAMESPACE

#endif // WLSUBSURFACE_H
