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
#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QWaylandShell;
class QWaylandShellSurface;

class WindowCompositorView : public QWaylandView
{
    Q_OBJECT
public:
    WindowCompositorView() : m_texture(0), m_shellSurface(0) {}
    GLuint getTexture();
    QPointF position() const { return m_position; }
    void setPosition(const QPointF &pos) { m_position = pos; }
    bool isCursor() const;
    bool hasShell() const { return m_shellSurface; }
private:
    friend class WindowCompositor;
    GLuint m_texture;
    QPointF m_position;
    QWaylandShellSurface *m_shellSurface;
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

    void handleMouseEvent(QWaylandView *target, QMouseEvent *me);
    void handleResize(WindowCompositorView *target, const QSize &initialSize, const QPoint &delta, int edge);

protected:
    void adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

signals:
    void startMove();
    void startResize(int edge);
    void frameOffset(const QPoint &offset);

private slots:
    void surfaceMappedChanged();
    void surfaceDestroyed();
    void surfaceCommittedSlot();
    void viewSurfaceDestroyed();
    void onStartResize(QWaylandInputDevice *inputDevice, QWaylandShellSurface::ResizeEdge edges);

    void triggerRender();

    void onSurfaceCreated(QWaylandSurface *surface);
    void onCreateShellSurface(QWaylandSurface *s, QWaylandClient *client, uint id);
    void updateCursor();
private:
    WindowCompositorView *findView(const QWaylandSurface *s) const;
    QWindow *m_window;
    QList<WindowCompositorView*> m_views;
    QWaylandShell *m_shell;

    QWaylandView m_cursorView;
    int m_cursorHotspotX;
    int m_cursorHotspotY;
};


QT_END_NAMESPACE

#endif // WINDOWCOMPOSITOR_H
