// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtCore/QPointer>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>

QT_BEGIN_NAMESPACE

class Window;
class QOpenGLTexture;
class QWaylandXdgShell;
class QWaylandXdgSurface;
class QWaylandXdgToplevel;

class View : public QWaylandView
{
    Q_OBJECT
public:
    explicit View() = default;
    QOpenGLTexture *getTexture();
    QSize bufferSize() const { return surface() ? surface()->bufferSize() : QSize(); }
    QRect globalGeometry() const { return {globalPosition(), surface()->bufferSize()}; }
    QPoint globalPosition() const { return m_globalPosition; }
    void setGlobalPosition(const QPoint &position);
    QPoint mapToLocal(const QPoint &globalPosition) const;
    void setAnchorEdges(Qt::Edges edges) { m_anchorEdges = edges; }
    void updateAnchoredPosition();

    virtual void handleResizeMove(const QPoint &delta);
    virtual void handleResizeRelease() {}

signals:
    void globalPositionChanged();
    void startResize();
    void startMove();

private:
    QOpenGLTexture *m_texture = nullptr;
    QPoint m_globalPosition;
    Qt::Edges m_anchorEdges;
    QSize m_lastSize;
};

class ToplevelView : public View
{
    Q_OBJECT
public:
    explicit ToplevelView(QWaylandXdgToplevel *toplevel);
    void handleResizeMove(const QPoint &delta) override;
    void handleResizeRelease() override;

private:
    QWaylandXdgToplevel *m_toplevel = nullptr;
    struct Resize  {
        QSize initialSize;
        Qt::Edges edges;
    } m_resize;
};

class Compositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    explicit Compositor() = default;
    ~Compositor() override;
    void create() override;
    void setWindow(Window *window) { m_window = window; }

    QList<View *> views() const { return m_views; }
    View *viewAt(const QPoint &position);

    void raise(View *view);

    void handleGlInitialized() { create(); }
    void handleMousePress(const QPoint &position, Qt::MouseButton button);
    void handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons);
    void handleMouseMove(const QPoint &position);
    void handleMouseWheel(Qt::Orientation orientation, int delta);

    void handleKeyPress(quint32 nativeScanCode);
    void handleKeyRelease(quint32 nativeScanCode);

signals:
    void startMove();

private slots:
    void handleXdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);
    void addView(View *view);
    void handleViewSurfaceDestroyed();
    void triggerRender();

private:
    Window *m_window = nullptr;
    QWaylandXdgShell *m_xdgShell = nullptr;
    QList<View *> m_views; // Sorted by painters algorithm (back to front)
    struct Grab {
        QPointer<View> view;
        enum State { None, Input, Move, Resize };
        State state = None;
        QPoint startLocalPosition; // in View's coordinate system
        QPoint startGlobalPosition;
    } m_grab;
};

QT_END_NAMESPACE

#endif // COMPOSITOR_H
