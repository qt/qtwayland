// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandxdgshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

QT_BEGIN_NAMESPACE

namespace QtWayland {

static void handlePopupCreated(QWaylandQuickShellSurfaceItem *parentItem, QWaylandXdgPopup *popup)
{
    if (parentItem->shellSurface() == popup->parentXdgSurface())
        QWaylandQuickShellSurfaceItemPrivate::get(parentItem)->maybeCreateAutoPopup(popup->xdgSurface());
}

XdgToplevelIntegration::XdgToplevelIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurface *>(item->shellSurface()))
    , m_toplevel(m_xdgSurface->toplevel())
    , grabberState(GrabberState::Default)
{
    Q_ASSERT(m_toplevel);

    m_item->setSurface(m_xdgSurface->surface());

    connect(m_toplevel, &QWaylandXdgToplevel::startMove, this, &XdgToplevelIntegration::handleStartMove);
    connect(m_toplevel, &QWaylandXdgToplevel::startResize, this, &XdgToplevelIntegration::handleStartResize);
    connect(m_toplevel, &QWaylandXdgToplevel::setMaximized, this, &XdgToplevelIntegration::handleSetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevel::unsetMaximized, this, &XdgToplevelIntegration::handleUnsetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevel::maximizedChanged, this, &XdgToplevelIntegration::handleMaximizedChanged);
    connect(m_toplevel, &QWaylandXdgToplevel::setFullscreen, this, &XdgToplevelIntegration::handleSetFullscreen);
    connect(m_toplevel, &QWaylandXdgToplevel::unsetFullscreen, this, &XdgToplevelIntegration::handleUnsetFullscreen);
    connect(m_toplevel, &QWaylandXdgToplevel::fullscreenChanged, this, &XdgToplevelIntegration::handleFullscreenChanged);
    connect(m_toplevel, &QWaylandXdgToplevel::activatedChanged, this, &XdgToplevelIntegration::handleActivatedChanged);
    connect(m_xdgSurface->shell(), &QWaylandXdgShell::popupCreated, this, [item](QWaylandXdgPopup *popup, QWaylandXdgSurface *){
        handlePopupCreated(item, popup);
    });
    connect(m_xdgSurface->surface(), &QWaylandSurface::destinationSizeChanged, this, &XdgToplevelIntegration::handleSurfaceSizeChanged);
    connect(m_toplevel, &QObject::destroyed, this, &XdgToplevelIntegration::handleToplevelDestroyed);
}

bool XdgToplevelIntegration::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        return filterMouseMoveEvent(mouseEvent);
    } else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
        return filterPointerReleaseEvent();
    } else if (event->type() == QEvent::TouchUpdate) {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        return filterTouchUpdateEvent(touchEvent);
    }
    return QWaylandQuickShellIntegration::eventFilter(object, event);
}

bool XdgToplevelIntegration::filterPointerMoveEvent(const QPointF &scenePosition)
{
    if (grabberState == GrabberState::Resize) {
        if (!resizeState.initialized) {
            resizeState.initialMousePos = scenePosition;
            resizeState.initialized = true;
            return true;
        }
        QPointF delta = m_item->mapToSurface(scenePosition - resizeState.initialMousePos);
        QSize newSize = m_toplevel->sizeForResize(resizeState.initialWindowSize, delta, resizeState.resizeEdges);
        m_toplevel->sendResizing(newSize);
    } else if (grabberState == GrabberState::Move) {
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
            moveState.initialOffset = moveItem->mapFromItem(nullptr, scenePosition);
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, scenePosition);
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }
    return false;
}

bool XdgToplevelIntegration::filterTouchUpdateEvent(QTouchEvent *event)
{
    if (event->pointCount() == 0)
        return false;

    Q_ASSERT(grabberState != GrabberState::Move || moveState.seat == m_item->compositor()->seatFor(event));
    Q_ASSERT(grabberState != GrabberState::Resize || resizeState.seat == m_item->compositor()->seatFor(event));

    QEventPoint point = event->points().first();
    return filterPointerMoveEvent(point.scenePosition());
 }

bool XdgToplevelIntegration::filterMouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(grabberState != GrabberState::Move || moveState.seat == m_item->compositor()->seatFor(event));
    Q_ASSERT(grabberState != GrabberState::Resize || resizeState.seat == m_item->compositor()->seatFor(event));

    return filterPointerMoveEvent(event->scenePosition());
}

bool XdgToplevelIntegration::filterPointerReleaseEvent()
{
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

void XdgToplevelIntegration::handleStartMove(QWaylandSeat *seat)
{
    grabberState = GrabberState::Move;
    moveState.seat = seat;
    moveState.initialized = false;
}

void XdgToplevelIntegration::handleStartResize(QWaylandSeat *seat, Qt::Edges edges)
{
    grabberState = GrabberState::Resize;
    resizeState.seat = seat;
    resizeState.resizeEdges = edges;
    resizeState.initialWindowSize = m_xdgSurface->windowGeometry().size();
    resizeState.initialPosition = m_item->moveItem()->position();
    resizeState.initialSurfaceSize = m_item->surface()->destinationSize();
    resizeState.initialized = false;
}

void XdgToplevelIntegration::handleSetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    QList<QWaylandXdgToplevel::State> states = m_toplevel->states();

    if (!states.contains(QWaylandXdgToplevel::State::FullscreenState) && !states.contains(QWaylandXdgToplevel::State::MaximizedState)) {
        windowedGeometry.initialWindowSize = m_xdgSurface->windowGeometry().size();
        windowedGeometry.initialPosition = m_item->moveItem()->position();
    }

    // Any prior output-resize handlers are irrelevant at this point.
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = m_item->view()->output();
    nonwindowedState.sizeChangedConnection = connect(nonwindowedState.output, &QWaylandOutput::availableGeometryChanged, this, &XdgToplevelIntegration::handleMaximizedSizeChanged);
    handleMaximizedSizeChanged();
}

void XdgToplevelIntegration::handleMaximizedSizeChanged()
{
    // Insurance against handleToplevelDestroyed() not managing to disconnect this
    // handler in time.
    if (m_toplevel == nullptr)
        return;

    m_toplevel->sendMaximized(nonwindowedState.output->availableGeometry().size() / nonwindowedState.output->scaleFactor());
}

void XdgToplevelIntegration::handleUnsetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    // If no prior windowed size was recorded, send a 0x0 configure event
    // to allow the client to choose its preferred size.
    if (windowedGeometry.initialWindowSize.isValid())
        m_toplevel->sendUnmaximized(windowedGeometry.initialWindowSize);
    else
        m_toplevel->sendUnmaximized();
}

void XdgToplevelIntegration::handleMaximizedChanged()
{
    if (m_toplevel->maximized()) {
        if (auto *output = m_item->view()->output()) {
            m_item->moveItem()->setPosition(output->position() + output->availableGeometry().topLeft());
        } else {
            qCWarning(qLcWaylandCompositor) << "The view does not have a corresponding output,"
                                            << "ignoring maximized state";
        }
    } else {
        m_item->moveItem()->setPosition(windowedGeometry.initialPosition);
    }
}

void XdgToplevelIntegration::handleSetFullscreen()
{
    if (!m_item->view()->isPrimary())
        return;

    QList<QWaylandXdgToplevel::State> states = m_toplevel->states();

    if (!states.contains(QWaylandXdgToplevel::State::FullscreenState) && !states.contains(QWaylandXdgToplevel::State::MaximizedState)) {
        windowedGeometry.initialWindowSize = m_xdgSurface->windowGeometry().size();
        windowedGeometry.initialPosition = m_item->moveItem()->position();
    }

    // Any prior output-resize handlers are irrelevant at this point.
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = m_item->view()->output();
    nonwindowedState.sizeChangedConnection = connect(nonwindowedState.output, &QWaylandOutput::geometryChanged, this, &XdgToplevelIntegration::handleFullscreenSizeChanged);
    handleFullscreenSizeChanged();
}

void XdgToplevelIntegration::handleFullscreenSizeChanged()
{
    // Insurance against handleToplevelDestroyed() not managing to disconnect this
    // handler in time.
    if (m_toplevel == nullptr)
        return;

    m_toplevel->sendFullscreen(nonwindowedState.output->geometry().size() / nonwindowedState.output->scaleFactor());
}

void XdgToplevelIntegration::handleUnsetFullscreen()
{
    if (!m_item->view()->isPrimary())
        return;

    // If no prior windowed size was recorded, send a 0x0 configure event
    // to allow the client to choose its preferred size.
    if (windowedGeometry.initialWindowSize.isValid())
        m_toplevel->sendUnmaximized(windowedGeometry.initialWindowSize);
    else
        m_toplevel->sendUnmaximized();
}

void XdgToplevelIntegration::handleFullscreenChanged()
{
    if (m_toplevel->fullscreen()) {
        if (auto *output = m_item->view()->output()) {
            m_item->moveItem()->setPosition(output->position() + output->geometry().topLeft());
        } else {
            qCWarning(qLcWaylandCompositor) << "The view does not have a corresponding output,"
                                            << "ignoring fullscreen state";
        }
    } else {
        m_item->moveItem()->setPosition(windowedGeometry.initialPosition);
    }
}

void XdgToplevelIntegration::handleActivatedChanged()
{
    if (m_toplevel->activated())
        m_item->raise();
}

void XdgToplevelIntegration::handleSurfaceSizeChanged()
{
    if (grabberState == GrabberState::Resize) {
        qreal dx = 0;
        qreal dy = 0;
        if (resizeState.resizeEdges & Qt::TopEdge)
            dy = resizeState.initialSurfaceSize.height() - m_item->surface()->destinationSize().height();
        if (resizeState.resizeEdges & Qt::LeftEdge)
            dx = resizeState.initialSurfaceSize.width() - m_item->surface()->destinationSize().width();
        QPointF offset = m_item->mapFromSurface({dx, dy});
        m_item->moveItem()->setPosition(resizeState.initialPosition + offset);
    }
}

void XdgToplevelIntegration::handleToplevelDestroyed()
{
    // Disarm any handlers that might fire on the now-stale toplevel pointer
    nonwindowedState.output = nullptr;
    disconnect(nonwindowedState.sizeChangedConnection);
}

XdgPopupIntegration::XdgPopupIntegration(QWaylandQuickShellSurfaceItem *item)
    : m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurface *>(item->shellSurface()))
    , m_popup(m_xdgSurface->popup())
{
    Q_ASSERT(m_popup);

    m_item->setSurface(m_xdgSurface->surface());
    handleGeometryChanged();

    connect(m_popup, &QWaylandXdgPopup::configuredGeometryChanged, this, &XdgPopupIntegration::handleGeometryChanged);
    connect(m_xdgSurface->shell(), &QWaylandXdgShell::popupCreated, this, [item](QWaylandXdgPopup *popup, QWaylandXdgSurface *){
        handlePopupCreated(item, popup);
    });
}

void XdgPopupIntegration::handleGeometryChanged()
{
    if (m_item->view()->output()) {
        const QPoint windowOffset = m_popup->parentXdgSurface()->windowGeometry().topLeft();
        const QPoint surfacePosition = m_popup->unconstrainedPosition() + windowOffset;
        const QPoint itemPosition = m_item->mapFromSurface(surfacePosition).toPoint();
        //TODO: positioner size or other size...?
        //TODO check positioner constraints etc... sliding, flipping
        m_item->moveItem()->setPosition(itemPosition);
    } else {
        qWarning() << "XdgPopupIntegration popup item without output" << m_item;
    }
}

}

QT_END_NAMESPACE

#include "moc_qwaylandxdgshellintegration_p.cpp"
