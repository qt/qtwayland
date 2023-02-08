// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "compositor.h"
#include "window.h"

#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandIviApplication>
#include <QtWaylandCompositor/QWaylandIviSurface>
#include <QtWaylandCompositor/QWaylandSeat>

#include <QRandomGenerator>
#include <QOpenGLFunctions>

//! [getTexture]
QOpenGLTexture *View::getTexture() {
    if (advance())
        m_texture = currentBuffer().toOpenGLTexture();
    return m_texture;
}
//! [getTexture]

QPoint View::mapToLocal(const QPoint &globalPos) const
{
    return globalPos - globalPosition();
}


// Normally, an IVI based compositor would have a design where each window has
// a defined position, based on the id. In this example, we just assign a random position.

void View::initPosition(const QSize &screenSize, const QSize &surfaceSize)
{
    if (m_positionSet)
        return;
    QRandomGenerator rand(iviId());
    int xrange = qMax(screenSize.width() - surfaceSize.width(), 1);
    int yrange = qMax(screenSize.height() - surfaceSize.height(), 1);
    setGlobalPosition(QPoint(rand.bounded(xrange), rand.bounded(yrange)));
}

Compositor::Compositor(Window *window)
    : m_window(window)
{
    window->setCompositor(this);
    connect(window, &Window::glReady, this, [this] { create(); });
}

Compositor::~Compositor()
{
}

//! [create]
void Compositor::create()
{
    QWaylandOutput *output = new QWaylandOutput(this, m_window);
    QWaylandOutputMode mode(m_window->size(), 60000);
    output->addMode(mode, true);
    QWaylandCompositor::create();
    output->setCurrentMode(mode);

    m_iviApplication = new QWaylandIviApplication(this);
    connect(m_iviApplication, &QWaylandIviApplication::iviSurfaceCreated, this, &Compositor::onIviSurfaceCreated);
}
//! [create]

View *Compositor::viewAt(const QPoint &position)
{
    // Since views are stored in painting order (back to front), we have to iterate backwards
    // to find the topmost view at a given point.
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
    m_views.append(view);
    defaultSeat()->setKeyboardFocus(view->surface());
    triggerRender();
}

static inline QPoint mapToView(const View *view, const QPoint &position)
{
    return view ? view->mapToLocal(position) : position;
}

//! [handleMousePress]
void Compositor::handleMousePress(const QPoint &position, Qt::MouseButton button)
{
    if (!m_mouseView) {
        if ((m_mouseView = viewAt(position)))
            raise(m_mouseView);
    }
    auto *seat = defaultSeat();
    seat->sendMouseMoveEvent(m_mouseView, mapToView(m_mouseView, position));
    seat->sendMousePressEvent(button);
}
//! [handleMousePress]

//! [handleMouseRelease]
void Compositor::handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons)
{
    auto *seat = defaultSeat();
    seat->sendMouseMoveEvent(m_mouseView, mapToView(m_mouseView, position));
    seat->sendMouseReleaseEvent(button);

    if (buttons == Qt::NoButton) {
        View *newView = viewAt(position);
        if (newView != m_mouseView)
            seat->sendMouseMoveEvent(newView, mapToView(newView, position));
        m_mouseView = nullptr;
    }
}
//! [handleMouseRelease]

void Compositor::handleMouseMove(const QPoint &position)
{
    View *view = m_mouseView ? m_mouseView.data() : viewAt(position);
    defaultSeat()->sendMouseMoveEvent(view, mapToView(view, position));
}

void Compositor::handleMouseWheel(const QPoint &angleDelta)
{
    // TODO: fix this to send a single event, when diagonal scrolling is supported
    if (angleDelta.x() != 0)
        defaultSeat()->sendMouseWheelEvent(Qt::Horizontal, angleDelta.x());
    if (angleDelta.y() != 0)
        defaultSeat()->sendMouseWheelEvent(Qt::Vertical, angleDelta.y());
}

void Compositor::handleKeyPress(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyPressEvent(nativeScanCode);
}

void Compositor::handleKeyRelease(quint32 nativeScanCode)
{
    defaultSeat()->sendKeyReleaseEvent(nativeScanCode);
}

//! [surfaceCreated]
void Compositor::onIviSurfaceCreated(QWaylandIviSurface *iviSurface)
{
    View *view = new View(iviSurface->iviId());
    view->setSurface(iviSurface->surface());
    view->setOutput(outputFor(m_window));

    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &Compositor::viewSurfaceDestroyed);
    connect(iviSurface->surface(), &QWaylandSurface::redraw, this, &Compositor::triggerRender);
}
//! [surfaceCreated]

//! [surfaceDestroyed]
void Compositor::viewSurfaceDestroyed()
{
    View *view = qobject_cast<View*>(sender());
    m_views.removeAll(view);
    delete view;
    triggerRender();
}
//! [surfaceDestroyed]

void Compositor::triggerRender()
{
    m_window->requestUpdate();
}

void Compositor::startRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->frameStarted();
}

void Compositor::endRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->sendFrameCallbacks();
}
