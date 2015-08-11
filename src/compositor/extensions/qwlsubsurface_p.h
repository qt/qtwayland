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

#include <QtCompositor/private/qwayland-server-sub-surface-extension.h>

#include <QtCompositor/QWaylandExtension>
#include <QtCompositor/QWaylandSurface>

#include <QtCore/QLinkedList>

QT_BEGIN_NAMESPACE

class Compositor;

namespace QtWayland {

class SubSurfaceExtensionGlobal : public QWaylandExtensionTemplate<SubSurfaceExtensionGlobal>, public QtWaylandServer::qt_sub_surface_extension
{
    Q_OBJECT
public:
    SubSurfaceExtensionGlobal(QWaylandCompositor *compositor);

private:
    QWaylandCompositor *m_compositor;

    void sub_surface_extension_get_sub_surface_aware_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) Q_DECL_OVERRIDE;
};

class Q_COMPOSITOR_EXPORT SubSurface : public QWaylandExtensionTemplate<SubSurface>, public QtWaylandServer::qt_sub_surface
{
    Q_OBJECT
    Q_PROPERTY(SubSurface *parent READ parent WRITE setParent NOTIFY parentChanged)
public:
    SubSurface(struct wl_client *client, uint32_t id, QWaylandSurface *surface);
    ~SubSurface();

    void setSubSurface(SubSurface *subSurface, int x, int y);
    void removeSubSurface(SubSurface *subSurfaces);

    SubSurface *parent() const;
    void setParent(SubSurface *parent);

    QLinkedList<QWaylandSurface *> subSurfaces() const;

    QWaylandSurface *surface() const;

Q_SIGNALS:
    void parentChanged(SubSurface *newParent, SubSurface *oldParent);

protected:
    void sub_surface_attach_sub_surface(Resource *resource, struct ::wl_resource *sub_surface, int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void sub_surface_move_sub_surface(Resource *resource, struct ::wl_resource *sub_surface, int32_t x, int32_t y) Q_DECL_OVERRIDE;
    void sub_surface_raise(Resource *resource, struct ::wl_resource *sub_surface) Q_DECL_OVERRIDE;
    void sub_surface_lower(Resource *resource, struct ::wl_resource *sub_surface) Q_DECL_OVERRIDE;

private:
    void parentDestroyed();
    struct wl_resource *m_sub_surface_resource;
    QWaylandSurface *m_surface;

    SubSurface *m_parent;
    QLinkedList<QWaylandSurface *> m_sub_surfaces;

};

inline QWaylandSurface *SubSurface::surface() const
{
    return m_surface;
}

}

QT_END_NAMESPACE

#endif // WLSUBSURFACE_H
