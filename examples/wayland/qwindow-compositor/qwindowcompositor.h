/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
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

#ifndef QWINDOWCOMPOSITOR_H
#define QWINDOWCOMPOSITOR_H

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandView>
#include "textureblitter.h"
#include "compositorwindow.h"

#include <QtGui/private/qopengltexturecache_p.h>
#include <QObject>
#include <QTimer>

QT_BEGIN_NAMESPACE

namespace QtWayland {
    class ExtendedSurface;
}

class QWaylandView;
class QWaylandShellSurface;
class QOpenGLTexture;

class QWindowCompositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    QWindowCompositor(CompositorWindow *window);
    ~QWindowCompositor();

    void create() Q_DECL_OVERRIDE;
private slots:
    void surfaceMappedChanged();
    void surfaceDestroyed();
    void surfaceCommittedSlot();
    void surfacePosChanged();

    void render();
    void onSurfaceCreated(QWaylandSurface *surface);
    void onShellSurfaceCreated(QWaylandSurface *surface, QWaylandShellSurface *shellSurface);
protected:
    QWaylandSurface *createSurface(QWaylandClient *client, quint32 id, int version) Q_DECL_OVERRIDE;
    void surfaceCommitted(QWaylandSurface *surface);

    QWaylandView* viewAt(const QPointF &point, QPointF *local = 0);

    bool eventFilter(QObject *obj, QEvent *event);
    QPointF toView(QWaylandView *view, const QPointF &pos) const;

    void adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

    void ensureKeyboardFocusSurface(QWaylandSurface *oldSurface);
    QImage makeBackgroundImage(const QString &fileName);

private slots:
    void extendedSurfaceCreated(QtWayland::ExtendedSurface *extSurface, QWaylandSurface *surface);
    void updateCursor(bool hasBuffer);

private:
    void surfaceMapped(QWaylandSurface *surface);
    void surfaceUnmapped(QWaylandSurface *surface);
    void drawSubSurface(const QPoint &offset, QWaylandSurface *surface);

    CompositorWindow *m_window;
    QImage m_backgroundImage;
    QOpenGLTexture *m_backgroundTexture;
    QList<QWaylandSurface *> m_visibleSurfaces;
    TextureBlitter *m_textureBlitter;
    GLuint m_surface_fbo;
    QTimer m_renderScheduler;

    //Dragging windows around
    QWaylandView *m_draggingWindow;
    bool m_dragKeyIsPressed;
    QPointF m_drag_diff;

    //Cursor
    QWaylandView m_cursorView;
    int m_cursorHotspotX;
    int m_cursorHotspotY;

    Qt::KeyboardModifiers m_modifiers;
};

QT_END_NAMESPACE

#endif // QWINDOWCOMPOSITOR_H
