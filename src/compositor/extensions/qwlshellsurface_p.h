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

#ifndef WLSHELLSURFACE_H
#define WLSHELLSURFACE_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandextension.h>

#include <wayland-server.h>
#include <QHash>
#include <QPoint>
#include <QSet>
#include <private/qwlpointer_p.h>

#include <QtCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QWaylandView;

namespace QtWayland {

class Compositor;
class Surface;
class ShellSurface;
class ShellSurfaceResizeGrabber;
class ShellSurfaceMoveGrabber;
class ShellSurfacePopupGrabber;

class Shell : public QWaylandExtensionTemplate<Shell>, public QtWaylandServer::wl_shell
{
    Q_OBJECT
public:
    Shell(QWaylandCompositor *compositor);

    ShellSurfacePopupGrabber* getPopupGrabber(QWaylandInputDevice *input);

private:
    void shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) Q_DECL_OVERRIDE;

    QHash<QWaylandInputDevice *, ShellSurfacePopupGrabber*> m_popupGrabber;
};

class Q_COMPOSITOR_EXPORT ShellSurface : public QWaylandExtensionTemplate<ShellSurface>, public QtWaylandServer::wl_shell_surface
{
    Q_OBJECT
    Q_PROPERTY(SurfaceType surfaceType READ surfaceType WRITE setSurfaceType NOTIFY surfaceTypeChanged)
public:
    enum SurfaceType {
        None,
        Toplevel,
        Transient,
        Popup
    };

    ShellSurface(Shell *shell, struct wl_client *client, uint32_t id, Surface *surface);
    ~ShellSurface();
    void sendConfigure(uint32_t edges, int32_t width, int32_t height);

    void adjustPosInResize();
    void resetResizeGrabber();
    void resetMoveGrabber();

    void setOffset(const QPointF &offset);

    void configure(bool hasBuffer);

    void requestSize(const QSize &size);

    Q_INVOKABLE void ping();
    void ping(uint32_t serial);

    QWaylandView *view() { return m_view; }
    void setView(QWaylandView *view) { m_view = view; }

    void setSurfaceType(SurfaceType type);
    SurfaceType surfaceType() const;

    bool isTransientInactive() const { return m_transientInactive; }

    QWaylandSurface *transientParent() const { return m_transientParent; }
    void setTransientParent(QWaylandSurface *parent) { m_transientParent = parent; }

    void setTransientOffset(const QPointF &offset) { m_transientOffset = offset; }
    QPointF transientOffset() const { return m_transientOffset; }

Q_SIGNALS:
    void surfaceTypeChanged();

private Q_SLOTS:
    void mapped();
    void adjustOffset(const QPoint &p);

private:
    Shell *m_shell;
    Surface *m_surface;
    QWaylandView *m_view;

    ShellSurfaceResizeGrabber *m_resizeGrabber;
    ShellSurfaceMoveGrabber *m_moveGrabber;
    ShellSurfacePopupGrabber *m_popupGrabber;

    uint32_t m_popupSerial;

    QSet<uint32_t> m_pings;

    SurfaceType m_surfaceType;
    bool m_transientInactive;

    QWaylandSurface *m_transientParent;
    QPointF m_transientOffset;

    void shell_surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void shell_surface_move(Resource *resource,
                            struct wl_resource *input_device_super,
                            uint32_t time) Q_DECL_OVERRIDE;
    void shell_surface_resize(Resource *resource,
                              struct wl_resource *input_device,
                              uint32_t time,
                              uint32_t edges) Q_DECL_OVERRIDE;
    void shell_surface_set_toplevel(Resource *resource) Q_DECL_OVERRIDE;
    void shell_surface_set_transient(Resource *resource,
                                     struct wl_resource *parent_surface_resource,
                                     int x,
                                     int y,
                                     uint32_t flags) Q_DECL_OVERRIDE;
    void shell_surface_set_fullscreen(Resource *resource,
                                      uint32_t method,
                                      uint32_t framerate,
                                      struct wl_resource *output_resource) Q_DECL_OVERRIDE;
    void shell_surface_set_popup(Resource *resource,
                                 struct wl_resource *input_device,
                                 uint32_t time,
                                 struct wl_resource *parent,
                                 int32_t x,
                                 int32_t y,
                                 uint32_t flags) Q_DECL_OVERRIDE;
    void shell_surface_set_maximized(Resource *resource,
                                     struct wl_resource *output_resource) Q_DECL_OVERRIDE;
    void shell_surface_pong(Resource *resource,
                            uint32_t serial) Q_DECL_OVERRIDE;
    void shell_surface_set_title(Resource *resource,
                                 const QString &title) Q_DECL_OVERRIDE;
    void shell_surface_set_class(Resource *resource,
                                 const QString &class_) Q_DECL_OVERRIDE;

    friend class ShellSurfaceMoveGrabber;
};

class ShellSurfaceGrabber : public QWaylandPointerGrabber
{
public:
    ShellSurfaceGrabber(ShellSurface *shellSurface);
    ~ShellSurfaceGrabber();

    ShellSurface *shell_surface;
};

class ShellSurfaceResizeGrabber : public ShellSurfaceGrabber
{
public:
    ShellSurfaceResizeGrabber(ShellSurface *shellSurface);

    QPointF point;
    enum wl_shell_surface_resize resize_edges;
    int32_t width;
    int32_t height;

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;
};

class ShellSurfaceMoveGrabber : public ShellSurfaceGrabber
{
public:
    ShellSurfaceMoveGrabber(ShellSurface *shellSurface, const QPointF &offset);

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QPointF m_offset;
};

class ShellSurfacePopupGrabber : public QWaylandDefaultPointerGrabber
{
public:
    ShellSurfacePopupGrabber(QWaylandInputDevice *inputDevice);

    uint32_t grabSerial() const;

    struct ::wl_client *client() const;
    void setClient(struct ::wl_client *client);

    void addPopup(ShellSurface *surface);
    void removePopup(ShellSurface *surface);

    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QWaylandInputDevice *m_inputDevice;
    struct ::wl_client *m_client;
    QList<ShellSurface *> m_surfaces;
    bool m_initialUp;
};

}

QT_END_NAMESPACE

#endif // WLSHELLSURFACE_H
