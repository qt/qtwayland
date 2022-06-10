// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "compositor.h"
#include "window.h"

#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandSeat>

#include <QOpenGLFunctions>

QOpenGLTexture *View::getTexture() {
    if (advance())
        m_texture = currentBuffer().toOpenGLTexture();
    return m_texture;
}

void View::setGlobalPosition(const QPoint &position)
{
    if (m_globalPosition == position)
        return;

    m_globalPosition = position;
    emit globalPositionChanged();
}

QPoint View::mapToLocal(const QPoint &globalPosition) const
{
    return globalPosition - this->globalPosition();
}

void View::updateAnchoredPosition()
{
    QPoint offset;
    QSize size = surface()->bufferSize();
    QSize delta = size - m_lastSize;
    if (m_anchorEdges & Qt::RightEdge)
        offset.setX(-delta.width());
    if (m_anchorEdges & Qt::BottomEdge)
        offset.setY(-delta.height());
    setGlobalPosition(globalPosition() + offset);
    m_lastSize = size;
}

void View::handleResizeMove(const QPoint &delta)
{
    Q_UNUSED(delta);
    qWarning() << "Resize not implemented for this view";
}

ToplevelView::ToplevelView(QWaylandXdgToplevel *toplevel)
    : m_toplevel(toplevel)
{
    QWaylandXdgSurface *xdgSurface = toplevel->xdgSurface();
    setSurface(xdgSurface->surface());
    connect(toplevel, &QWaylandXdgToplevel::startMove, this, &View::startMove);
    connect(toplevel, &QWaylandXdgToplevel::startResize, this, [this](QWaylandSeat *, Qt::Edges edges) {
        m_resize.edges = edges;
        m_resize.initialSize = m_toplevel->xdgSurface()->windowGeometry().size();
        Qt::Edges opposite = edges ^ Qt::Edges(0b1111);
        setAnchorEdges(opposite);
        emit startResize();
    });
    QList<QWaylandXdgToplevel::State> states{QWaylandXdgToplevel::ActivatedState};
    toplevel->sendConfigure(QSize(0, 0), states);
}

void ToplevelView::handleResizeMove(const QPoint &delta)
{
    QSize newSize = m_toplevel->sizeForResize(m_resize.initialSize, delta, m_resize.edges);
    m_toplevel->sendResizing(newSize);
}

void ToplevelView::handleResizeRelease()
{
    setAnchorEdges({});
    m_resize.edges = {};
    m_resize.initialSize = {};
}

Compositor::~Compositor()
{
    delete m_xdgShell;
}

void Compositor::create()
{
    QWaylandCompositor::create();

    m_xdgShell = new QWaylandXdgShell(this);
    connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated, this, &Compositor::handleXdgToplevelCreated);
}

View *Compositor::viewAt(const QPoint &position)
{
    // Since views are stored in painting order (back to front), we have to iterate backwards
    // to find the topmost view at the given point
    for (auto it = m_views.crbegin(); it != m_views.crend(); ++it) {
        View *view = *it;
        if (view->globalGeometry().contains(position))
            return view;
    }
    return nullptr;
}

void Compositor::raise(View *view)
{
    m_views.removeAll(view);
    m_views << view;
    defaultSeat()->setKeyboardFocus(view->surface());
    triggerRender();
}

void Compositor::handleMousePress(const QPoint &position, Qt::MouseButton button)
{
    if (m_grab.state == Grab::None) {
        m_grab.view = viewAt(position);
        if (m_grab.view) {
            m_grab.state = Grab::Input;
            m_grab.startGlobalPosition = position;
            m_grab.startLocalPosition = m_grab.view->mapToLocal(position);
            raise(m_grab.view);
        }
    }

    switch (m_grab.state) {
    case Grab::Input: {
        auto *seat = defaultSeat();
        seat->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        seat->sendMousePressEvent(button);
        break;
    }
    case Grab::Move:
    case Grab::Resize:
    case Grab::None:
        break;
    }
}

void Compositor::handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons)
{
    auto *seat = defaultSeat();

    switch (m_grab.state) {
    case Grab::Input:
        seat->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        seat->sendMouseReleaseEvent(button);
        if (buttons == Qt::NoButton) {
            View *newView = viewAt(position);
            if (newView != m_grab.view) {
                seat->setMouseFocus(newView);
                if (newView)
                    seat->sendMouseMoveEvent(newView, newView->mapToLocal(position));
            }
            m_grab.view = nullptr;
            m_grab.state = Grab::None;
        }
        break;
    case Grab::Move:
    case Grab::Resize:
        m_grab.state = Grab::None;
        m_grab.view = nullptr;
        if (View *view = viewAt(position))
            seat->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    case Grab::None:
        if (View *view = viewAt(position))
            seat->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    }
}

void Compositor::handleMouseMove(const QPoint &position)
{
    switch (m_grab.state) {
    case Grab::Input:
        defaultSeat()->sendMouseMoveEvent(m_grab.view, m_grab.view->mapToLocal(position));
        break;
    case Grab::None:
        if (View *view = viewAt(position))
            defaultSeat()->sendMouseMoveEvent(view, view->mapToLocal(position));
        break;
    case Grab::Resize:
        m_grab.view->handleResizeMove(position - m_grab.startGlobalPosition);
        break;
    case Grab::Move:
        m_grab.view->setGlobalPosition(position - m_grab.startLocalPosition);
        break;
    }
}

void Compositor::handleMouseWheel(Qt::Orientation orientation, int delta)
{
    defaultSeat()->sendMouseWheelEvent(orientation, delta);
}

void Compositor::handleKeyPress(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyPressEvent(nativeScanCode);
}

void Compositor::handleKeyRelease(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyReleaseEvent(nativeScanCode);
}

void Compositor::handleXdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface)
{
    Q_UNUSED(xdgSurface);
    auto *view = new ToplevelView(toplevel);
    addView(view);
}

void Compositor::addView(View *view)
{
    view->setOutput(outputFor(m_window));
    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &Compositor::handleViewSurfaceDestroyed);
    connect(view, &View::globalPositionChanged, this, &Compositor::triggerRender);
    connect(view->surface(), &QWaylandSurface::redraw, this, &Compositor::triggerRender);
    connect(view, &View::startMove, this, [this, view](){
        m_grab.view = view;
        m_grab.state = Grab::Move;
    });
    connect(view, &View::startResize, this, [this, view]() {
        m_grab.view = view;
        m_grab.state = Grab::Resize;
    });
}

void Compositor::handleViewSurfaceDestroyed()
{
    auto *view = qobject_cast<ToplevelView*>(sender());
    m_views.removeAll(view);
    delete view;
    triggerRender();
}

void Compositor::triggerRender()
{
    m_window->requestUpdate();
}
