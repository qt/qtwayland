/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Wayland module
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

#ifndef WINDOWCOMPOSITOR_H
#define WINDOWCOMPOSITOR_H

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QWaylandWlShell;
class QWaylandWlShellSurface;
class QWaylandXdgShell;

class WindowCompositorView : public QWaylandView
{
    Q_OBJECT
public:
    WindowCompositorView();
    GLuint getTexture(GLenum *target = 0);
    QPointF position() const { return m_position; }
    void setPosition(const QPointF &pos) { m_position = pos; }
    bool isCursor() const;
    bool hasShell() const { return m_wlShellSurface; }
    void setParentView(WindowCompositorView *parent) { m_parentView = parent; }
    WindowCompositorView *parentView() const { return m_parentView; }
    QPointF parentPosition() const { return m_parentView ? (m_parentView->position() + m_parentView->parentPosition()) : QPointF(); }
    QSize windowSize() { return m_xdgSurface ? m_xdgSurface->windowGeometry().size() : surface()->size(); }
    QPoint offset() const { return m_offset; }

private:
    friend class WindowCompositor;
    GLenum m_textureTarget;
    GLuint m_texture;
    QPointF m_position;
    QWaylandWlShellSurface *m_wlShellSurface;
    QWaylandXdgSurface *m_xdgSurface;
    QWaylandXdgPopup *m_xdgPopup;
    WindowCompositorView *m_parentView;
    QPoint m_offset;

public slots:
    void onXdgSetMaximized();
    void onXdgUnsetMaximized();
    void onXdgSetFullscreen(QWaylandOutput *output);
    void onXdgUnsetFullscreen();
    void onOffsetForNextFrame(const QPoint &offset);
};

class WindowCompositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    WindowCompositor(QWindow *window);
    ~WindowCompositor();
    void create() Q_DECL_OVERRIDE;

    void startRender();
    void endRender();

    QList<WindowCompositorView*> views() const { return m_views; }
    void raise(WindowCompositorView *view);

    void handleMouseEvent(QWaylandView *target, QMouseEvent *me);
    void handleResize(WindowCompositorView *target, const QSize &initialSize, const QPoint &delta, int edge);
    void handleDrag(WindowCompositorView *target, QMouseEvent *me);

    bool popupActive() const { return !m_popupViews.isEmpty(); }
    void closePopups();
protected:
    void adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

signals:
    void startMove();
    void startResize(int edge, bool anchored);
    void dragStarted(WindowCompositorView *dragIcon);
    void frameOffset(const QPoint &offset);

private slots:
    void surfaceMappedChanged();
    void surfaceDestroyed();
    void viewSurfaceDestroyed();
    void onStartMove();
    void onWlStartResize(QWaylandInputDevice *inputDevice, QWaylandWlShellSurface::ResizeEdge edges);
    void onXdgStartResize(QWaylandInputDevice *inputDevice, QWaylandXdgSurface::ResizeEdge edges);

    void startDrag();

    void triggerRender();

    void onSurfaceCreated(QWaylandSurface *surface);
    void onWlShellSurfaceCreated(QWaylandWlShellSurface *wlShellSurface);
    void onXdgSurfaceCreated(QWaylandXdgSurface *xdgSurface);
    void onCreateXdgPopup(QWaylandSurface *surface, QWaylandSurface *parent, QWaylandInputDevice *inputDevice,
                          const QPoint &position, const QWaylandResource &resource);
    void onSetTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, QWaylandWlShellSurface::FocusPolicy focusPolicy);
    void onSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent);

    void onSubsurfaceChanged(QWaylandSurface *child, QWaylandSurface *parent);
    void onSubsurfacePositionChanged(const QPoint &position);

    void updateCursor();
private:
    WindowCompositorView *findView(const QWaylandSurface *s) const;
    QWindow *m_window;
    QList<WindowCompositorView*> m_views;
    QList<WindowCompositorView*> m_popupViews;
    QWaylandWlShell *m_wlShell;
    QWaylandXdgShell *m_xdgShell;
    QWaylandView m_cursorView;
    int m_cursorHotspotX;
    int m_cursorHotspotY;
};


QT_END_NAMESPACE

#endif // WINDOWCOMPOSITOR_H
