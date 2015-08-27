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

#ifndef QWAYLANDSHELL_P_H
#define QWAYLANDSHELL_P_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/private/qwaylandextension_p.h>
#include <QtCompositor/QWaylandPointerGrabber>
#include <QtCompositor/QWaylandShellSurface>
#include <QtCompositor/QWaylandInputDevice>

#include <wayland-server.h>
#include <QHash>
#include <QPoint>
#include <QSet>

#include <QtCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QWaylandView;

class QWaylandShellSurfaceResizeGrabber;
class QWaylandShellSurfaceMoveGrabber;
class QWaylandShellSurfacePopupGrabber;

class Q_COMPOSITOR_EXPORT QWaylandShellPrivate
                                        : public QWaylandExtensionTemplatePrivate
                                        , public QtWaylandServer::wl_shell
{
    Q_DECLARE_PUBLIC(QWaylandShell)
public:
    QWaylandShellPrivate();
    static QWaylandShellPrivate *get(QWaylandShell *shell) { return shell->d_func(); }

    QWaylandShellSurfacePopupGrabber* getPopupGrabber(QWaylandInputDevice *input);
protected:
    void shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) Q_DECL_OVERRIDE;

    QHash<QWaylandInputDevice *, QWaylandShellSurfacePopupGrabber*> m_popupGrabber;
};

class Q_COMPOSITOR_EXPORT QWaylandShellSurfacePrivate
                                        : public QWaylandExtensionTemplatePrivate
                                        , public QtWaylandServer::wl_shell_surface
{
    Q_DECLARE_PUBLIC(QWaylandShellSurface)
public:
    QWaylandShellSurfacePrivate();
    ~QWaylandShellSurfacePrivate();

    static QWaylandShellSurfacePrivate *get(QWaylandShellSurface *surface) { return surface->d_func(); }

    void resetResizeGrabber();
    void resetMoveGrabber();

    void setSurfaceType(QWaylandShellSurface::SurfaceType type);

    void setOffset(const QPointF &offset) { m_transientOffset = offset; }

    void requestSize(const QSize &size);

    void ping();
    void ping(uint32_t serial);

private:
    QWaylandShell *m_shell;
    QWaylandSurface *m_surface;
    QWaylandView *m_view;

    QWaylandShellSurfaceResizeGrabber *m_resizeGrabber;
    QWaylandShellSurfaceMoveGrabber *m_moveGrabber;
    QWaylandShellSurfacePopupGrabber *m_popupGrabber;

    uint32_t m_popupSerial;

    QSet<uint32_t> m_pings;

    QWaylandShellSurface::SurfaceType m_surfaceType;
    bool m_transientInactive;

    QWaylandSurface *m_transientParent;
    QPointF m_transientOffset;

    QString m_title;
    QString m_className;

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

};

class QWaylandShellSurfaceGrabber : public QWaylandPointerGrabber
{
public:
    QWaylandShellSurfaceGrabber(QWaylandShellSurface *shellSurface);
    ~QWaylandShellSurfaceGrabber();

    QWaylandShellSurface *shell_surface;
};

class QWaylandShellSurfaceResizeGrabber : public QWaylandShellSurfaceGrabber
{
public:
    QWaylandShellSurfaceResizeGrabber(QWaylandShellSurface *shellSurface);

    QPointF point;
    enum wl_shell_surface_resize resize_edges;
    int32_t width;
    int32_t height;

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;
};

class QWaylandShellSurfaceMoveGrabber : public QWaylandShellSurfaceGrabber
{
public:
    QWaylandShellSurfaceMoveGrabber(QWaylandShellSurface *shellSurface, const QPointF &offset);

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QPointF m_offset;
};

class QWaylandShellSurfacePopupGrabber : public QWaylandDefaultPointerGrabber
{
public:
    QWaylandShellSurfacePopupGrabber(QWaylandInputDevice *inputDevice);

    uint32_t grabSerial() const { return m_inputDevice->pointer()->grabSerial(); }

    struct ::wl_client *client() const { return m_client; }
    void setClient(struct ::wl_client *client) { m_client = client; }

    void addPopup(QWaylandShellSurface *surface);
    void removePopup(QWaylandShellSurface *surface);

    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

private:
    QWaylandInputDevice *m_inputDevice;
    struct ::wl_client *m_client;
    QList<QWaylandShellSurface *> m_surfaces;
    bool m_initialUp;
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELL_P_H
