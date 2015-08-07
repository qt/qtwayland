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

#ifndef WL_SURFACE_H
#define WL_SURFACE_H

#include <QtCompositor/qwaylandexport.h>

#include <private/qwlsurfacebuffer_p.h>
#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandbufferref.h>

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

class QTouchEvent;

namespace QtWayland {

class Compositor;
class Buffer;
class ExtendedSurface;
class InputPanelSurface;
class SubSurface;
class FrameCallback;

class Q_COMPOSITOR_EXPORT Surface : public QtWaylandServer::wl_surface
{
public:
    Surface(struct wl_client *client, uint32_t id, int version, QWaylandCompositor *compositor, QWaylandSurface *surface);
    ~Surface();

    static Surface *fromResource(struct ::wl_resource *resource);

    bool mapped() const;

    using QtWaylandServer::wl_surface::resource;

    QSize size() const;
    void setSize(const QSize &size);

    QRegion inputRegion() const;
    QRegion opaqueRegion() const;

    void sendFrameCallback();
    void removeFrameCallback(FrameCallback *callback);

    QWaylandSurface *waylandSurface() const;

    QPoint lastMousePos() const;

    void setSubSurface(SubSurface *subSurface);
    SubSurface *subSurface() const;

    void setInputPanelSurface(InputPanelSurface *inputPanelSurface);
    InputPanelSurface *inputPanelSurface() const;

    Compositor *compositor() const;

    QString className() const { return m_className; }
    void setClassName(const QString &className);

    QString title() const { return m_title; }
    void setTitle(const QString &title);

    bool isCursorSurface() const { return m_isCursorSurface; }
    void setCursorSurface(bool isCursor) { m_isCursorSurface = isCursor; }

    void frameStarted();

    void setMapped(bool mapped);
    void setVisibility(QWindow::Visibility visibility) { m_visibility = visibility; }

    inline bool isDestroyed() const { return m_destroyed; }

    Qt::ScreenOrientation contentOrientation() const;

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

    Q_DISABLE_COPY(Surface)

    Compositor *m_compositor;
    QWaylandSurface *m_waylandSurface;

    QRegion m_damage;
    SurfaceBuffer *m_buffer;
    QWaylandBufferRef m_bufferRef;
    bool m_surfaceMapped;

    struct {
        SurfaceBuffer *buffer;
        QRegion damage;
        QPoint offset;
        bool newlyAttached;
        QRegion inputRegion;
    } m_pending;

    QPoint m_lastLocalMousePos;
    QPoint m_lastGlobalMousePos;

    QList<FrameCallback *> m_pendingFrameCallbacks;
    QList<FrameCallback *> m_frameCallbacks;

    SubSurface *m_subSurface;
    InputPanelSurface *m_inputPanelSurface;

    QRegion m_inputRegion;
    QRegion m_opaqueRegion;

    QVector<SurfaceBuffer *> m_bufferPool;

    QSize m_size;
    QString m_className;
    QString m_title;
    bool m_isCursorSurface;
    bool m_destroyed;
    Qt::ScreenOrientation m_contentOrientation;
    QWindow::Visibility m_visibility;

    void setBackBuffer(SurfaceBuffer *buffer, const QRegion &damage);

    SurfaceBuffer *createSurfaceBuffer(struct ::wl_resource *buffer);

    friend class QWaylandSurface;
};

}

QT_END_NAMESPACE

#endif //WL_SURFACE_H
