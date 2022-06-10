// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDWLSHELLINTEGRATION_H
#define QWAYLANDWLSHELLINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>

#include <QtWaylandCompositor/QWaylandWlShellSurface>

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

namespace QtWayland {

class WlShellIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    WlShellIntegration(QWaylandQuickShellSurfaceItem *item);
    ~WlShellIntegration() override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void handleStartMove(QWaylandSeat *seat);
    void handleStartResize(QWaylandSeat *seat, QWaylandWlShellSurface::ResizeEdge edges);
    void handleSetDefaultTopLevel();
    void handleSetTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive);
    void handleSetMaximized(QWaylandOutput *output);
    void handleSetFullScreen(QWaylandWlShellSurface::FullScreenMethod method, uint framerate, QWaylandOutput *output);
    void handleSetPopup(QWaylandSeat *seat, QWaylandSurface *parent, const QPoint &relativeToParent);
    void handleShellSurfaceDestroyed();
    void handleSurfaceHasContentChanged();
    void handleRedraw();
    void adjustOffsetForNextFrame(const QPointF &offset);
    void handleFullScreenSizeChanged();
    void handleMaximizedSizeChanged();

private:
    enum class GrabberState {
        Default,
        Resize,
        Move
    };

    void handlePopupClosed();
    void handlePopupRemoved();
    qreal devicePixelRatio() const;

    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QPointer<QWaylandWlShellSurface> m_shellSurface;
    GrabberState grabberState = GrabberState::Default;
    struct {
        QWaylandSeat *seat = nullptr;
        QPointF initialOffset;
        bool initialized = false;
    } moveState;
    struct {
        QWaylandSeat *seat = nullptr;
        QWaylandWlShellSurface::ResizeEdge resizeEdges;
        QSizeF initialSize;
        QPointF initialMousePos;
        bool initialized = false;
    } resizeState;

    bool isPopup = false;

    enum class State {
        Windowed,
        Maximized,
        FullScreen
    };

    State currentState = State::Windowed;
    State nextState = State::Windowed;

    struct {
        QWaylandOutput *output = nullptr;
        QMetaObject::Connection sizeChangedConnection; // Depending on whether maximized or fullscreen,
                                                       // will be hooked to geometry-changed or available-
                                                       // geometry-changed.
    } nonwindowedState;

    QPointF normalPosition;
    QPointF finalPosition;

    bool filterMouseMoveEvent(QMouseEvent *event);
    bool filterMouseReleaseEvent(QMouseEvent *event);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDWLSHELLINTEGRATION_H
