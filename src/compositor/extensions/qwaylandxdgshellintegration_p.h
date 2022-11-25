// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGSHELLINTEGRATION_H
#define QWAYLANDXDGSHELLINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QWaylandXdgSurface;

namespace QtWayland {

class XdgToplevelIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgToplevelIntegration(QWaylandQuickShellSurfaceItem *item);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void handleStartMove(QWaylandSeat *seat);
    void handleStartResize(QWaylandSeat *seat, Qt::Edges edges);
    void handleSetMaximized();
    void handleUnsetMaximized();
    void handleMaximizedChanged();
    void handleSetFullscreen();
    void handleUnsetFullscreen();
    void handleFullscreenChanged();
    void handleActivatedChanged();
    void handleSurfaceSizeChanged();
    void handleToplevelDestroyed();
    void handleMaximizedSizeChanged();
    void handleFullscreenSizeChanged();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgToplevel *m_toplevel = nullptr;

    enum class GrabberState {
        Default,
        Resize,
        Move
    };
    GrabberState grabberState;

    struct {
        QWaylandSeat *seat = nullptr;
        QPointF initialOffset;
        bool initialized;
    } moveState;

    struct {
        QWaylandSeat *seat = nullptr;
        Qt::Edges resizeEdges;
        QSizeF initialWindowSize;
        QPointF initialMousePos;
        QPointF initialPosition;
        QSize initialSurfaceSize;
        bool initialized;
    } resizeState;

    struct {
        QSize initialWindowSize;
        QPointF initialPosition;
    } windowedGeometry;

    struct {
        QWaylandOutput *output = nullptr;
        QMetaObject::Connection sizeChangedConnection; // Depending on whether maximized or fullscreen,
                                                       // will be hooked to geometry-changed or available-
                                                       // geometry-changed.
    } nonwindowedState;

    bool filterPointerMoveEvent(const QPointF &scenePosition);
    bool filterMouseMoveEvent(QMouseEvent *event);
    bool filterPointerReleaseEvent();
    bool filterTouchUpdateEvent(QTouchEvent *event);
};

class XdgPopupIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgPopupIntegration(QWaylandQuickShellSurfaceItem *item);

private Q_SLOTS:
    void handleGeometryChanged();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandXdgSurface *m_xdgSurface = nullptr;
    QWaylandXdgPopup *m_popup = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELLINTEGRATION_H
