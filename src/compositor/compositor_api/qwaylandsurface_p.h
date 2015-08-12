/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QWAYLANDSURFACE_P_H
#define QWAYLANDSURFACE_P_H

#include <QtCompositor/qwaylandexport.h>
#include <private/qobject_p.h>

#include <private/qwlsurfacebuffer_p.h>
#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandbufferref.h>

#include <QtCompositor/private/qwlinputpanelsurface_p.h>
#include <QtCompositor/private/qwlregion_p.h>

#include <QtCore/QVector>
#include <QtCore/QRect>
#include <QtGui/QRegion>
#include <QtGui/QImage>
#include <QtGui/QWindow>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>

#include <wayland-util.h>

#include <QtCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandSurface;
class QWaylandView;
class QWaylandSurfaceInterface;

namespace QtWayland {
class FrameCallback;
}

class Q_COMPOSITOR_EXPORT QWaylandSurfacePrivate : public QObjectPrivate, public QtWaylandServer::wl_surface
{
public:
    static QWaylandSurfacePrivate *get(QWaylandSurface *surface);

    QWaylandSurfacePrivate(QWaylandClient *client, quint32 id, int version, QWaylandCompositor *compositor);
    ~QWaylandSurfacePrivate();

    void refView(QWaylandView *view);
    void derefView(QWaylandView *view);

    static QWaylandSurfacePrivate *fromResource(struct ::wl_resource *resource)
    { return static_cast<QWaylandSurfacePrivate *>(Resource::fromResource(resource)->surface_object); }

    bool mapped() const { return QtWayland::SurfaceBuffer::hasContent(m_buffer); }

    using QtWaylandServer::wl_surface::resource;

    QSize size() const { return m_size; }
    void setSize(const QSize &size);

    QRegion inputRegion() const { return m_inputRegion; }
    QRegion opaqueRegion() const { return m_opaqueRegion; }

    void sendFrameCallback();
    void removeFrameCallback(QtWayland::FrameCallback *callback);

    QPoint lastMousePos() const { return m_lastLocalMousePos; }

    void setInputPanelSurface(QtWayland::InputPanelSurface *inputPanelSurface) { m_inputPanelSurface = inputPanelSurface; }
    QtWayland::InputPanelSurface *inputPanelSurface() const { return m_inputPanelSurface; }

    QWaylandCompositor *compositor() const { return m_compositor; }

    bool isCursorSurface() const { return m_isCursorSurface; }
    void setCursorSurface(bool isCursor) { m_isCursorSurface = isCursor; }

    void frameStarted();

    inline bool isDestroyed() const { return m_destroyed; }

    Qt::ScreenOrientation contentOrientation() const { return m_contentOrientation; }

    QWaylandSurface::Origin origin() const { return m_buffer ? m_buffer->origin() : QWaylandSurface::OriginTopLeft; }

    QWaylandBufferRef currentBufferRef() const { return m_bufferRef; }

    void notifyViewsAboutDestruction();
protected:
    void surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void surface_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void surface_attach(Resource *resource,
                        struct wl_resource *buffer, int x, int y) Q_DECL_OVERRIDE;
    void surface_damage(Resource *resource,
                        int32_t x, int32_t y, int32_t width, int32_t height) Q_DECL_OVERRIDE;
    void surface_frame(Resource *resource,
                       uint32_t callback) Q_DECL_OVERRIDE;
    void surface_set_opaque_region(Resource *resource,
                                   struct wl_resource *region) Q_DECL_OVERRIDE;
    void surface_set_input_region(Resource *resource,
                                  struct wl_resource *region) Q_DECL_OVERRIDE;
    void surface_commit(Resource *resource) Q_DECL_OVERRIDE;
    void surface_set_buffer_transform(Resource *resource, int32_t transform) Q_DECL_OVERRIDE;

    void setBackBuffer(QtWayland::SurfaceBuffer *buffer, const QRegion &damage);
    QtWayland::SurfaceBuffer *createSurfaceBuffer(struct ::wl_resource *buffer);

protected: //member variables
    QWaylandCompositor *m_compositor;
    int refCount;
    QWaylandClient *client;
    QList<QWaylandView *> views;
    QRegion m_damage;
    QtWayland::SurfaceBuffer *m_buffer;
    QWaylandBufferRef m_bufferRef;

    struct {
        QtWayland::SurfaceBuffer *buffer;
        QRegion damage;
        QPoint offset;
        bool newlyAttached;
        QRegion inputRegion;
    } m_pending;

    QPoint m_lastLocalMousePos;
    QPoint m_lastGlobalMousePos;

    QList<QtWayland::FrameCallback *> m_pendingFrameCallbacks;
    QList<QtWayland::FrameCallback *> m_frameCallbacks;

    QtWayland::InputPanelSurface *m_inputPanelSurface;

    QRegion m_inputRegion;
    QRegion m_opaqueRegion;

    QVector<QtWayland::SurfaceBuffer *> m_bufferPool;

    QSize m_size;
    bool m_isCursorSurface;
    bool m_destroyed;
    Qt::ScreenOrientation m_contentOrientation;
    QWindow::Visibility m_visibility;

    Q_DECLARE_PUBLIC(QWaylandSurface)
    Q_DISABLE_COPY(QWaylandSurfacePrivate)
};

QT_END_NAMESPACE

#endif
