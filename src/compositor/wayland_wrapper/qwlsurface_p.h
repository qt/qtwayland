/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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

#ifndef WL_SURFACE_H
#define WL_SURFACE_H

#include <QtCompositor/qwaylandexport.h>

#include <private/qwlsurfacebuffer_p.h>
#include <QtCompositor/qwaylandsurface.h>

#include <QtCore/QVector>
#include <QtCore/QRect>
#include <QtGui/QImage>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

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
class ShellSurface;

class Q_COMPOSITOR_EXPORT Surface : public QtWaylandServer::wl_surface
{
public:
    Surface(struct wl_client *client, uint32_t id, Compositor *compositor);
    ~Surface();

    static Surface *fromResource(struct ::wl_resource *resource);

    QWaylandSurface::Type type() const;
    bool isYInverted() const;

    bool visible() const;

    using QtWaylandServer::wl_surface::resource;

    QPointF pos() const;
    QPointF nonAdjustedPos() const;
    void setPos(const QPointF  &pos);

    QSize size() const;
    void setSize(const QSize &size);

    QRegion inputRegion() const;
    QRegion opaqueRegion() const;

    QImage image() const;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint textureId() const;
#endif

    void sendFrameCallback();

    void frameFinished();

    QWaylandSurface *waylandSurface() const;

    QPoint lastMousePos() const;

    void setExtendedSurface(ExtendedSurface *extendedSurface);
    ExtendedSurface *extendedSurface() const;

    void setSubSurface(SubSurface *subSurface);
    SubSurface *subSurface() const;

    void setShellSurface(ShellSurface *shellSurface);
    ShellSurface *shellSurface() const;

    void setInputPanelSurface(InputPanelSurface *inputPanelSurface);
    InputPanelSurface *inputPanelSurface() const;

    Compositor *compositor() const;

    QString className() const { return m_className; }
    void setClassName(const QString &className);

    QString title() const { return m_title; }
    void setTitle(const QString &title);

    bool transientInactive() const { return m_transientInactive; }
    void setTransientInactive(bool v) { m_transientInactive = v; }

    bool isCursorSurface() const { return m_isCursorSurface; }
    void setCursorSurface(bool isCursor) { m_isCursorSurface = isCursor; }

    void advanceBufferQueue();
    void releaseSurfaces();

private:
    Q_DISABLE_COPY(Surface)

    Compositor *m_compositor;
    QWaylandSurface *m_waylandSurface;

    SurfaceBuffer *m_backBuffer;
    SurfaceBuffer *m_frontBuffer;
    QList<SurfaceBuffer *> m_bufferQueue;
    bool m_surfaceMapped;

    QPoint m_lastLocalMousePos;
    QPoint m_lastGlobalMousePos;

    struct wl_list m_frame_callback_list;

    ExtendedSurface *m_extendedSurface;
    SubSurface *m_subSurface;
    ShellSurface *m_shellSurface;
    InputPanelSurface *m_inputPanelSurface;

    QRegion m_inputRegion;
    QRegion m_opaqueRegion;

    QVector<SurfaceBuffer *> m_bufferPool;

    QPointF m_position;
    QSize m_size;
    QString m_className;
    QString m_title;
    bool m_transientInactive;
    bool m_isCursorSurface;

    inline SurfaceBuffer *currentSurfaceBuffer() const;
    void damage(const QRect &rect);
    void setBackBuffer(SurfaceBuffer *buffer);
    SurfaceBuffer *createSurfaceBuffer(struct ::wl_resource *buffer);

    void attach(struct ::wl_resource *buffer);

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

};

inline SurfaceBuffer *Surface::currentSurfaceBuffer() const {
    return m_backBuffer? m_backBuffer : m_frontBuffer;
}

}

QT_END_NAMESPACE

#endif //WL_SURFACE_H
