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

#include "windowcompositor.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>

#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/qwaylandinput.h>
#include <QtWaylandCompositor/qwaylanddrag.h>

#include <QDebug>
#include <QOpenGLContext>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

WindowCompositorView::WindowCompositorView()
    : m_textureTarget(GL_TEXTURE_2D)
    , m_texture(0)
    , m_wlShellSurface(nullptr)
    , m_xdgSurface(nullptr)
    , m_xdgPopup(nullptr)
    , m_parentView(nullptr)
{}

GLuint WindowCompositorView::getTexture(GLenum *target)
{
    QWaylandBufferRef buf = currentBuffer();
    GLuint streamingTexture = buf.textureForPlane(0);
    if (streamingTexture)
        m_texture = streamingTexture;

    if (!buf.isShm() && buf.bufferFormatEgl() == QWaylandBufferRef::BufferFormatEgl_EXTERNAL_OES)
        m_textureTarget = GL_TEXTURE_EXTERNAL_OES;

    if (advance()) {
        buf = currentBuffer();
        if (!m_texture)
            glGenTextures(1, &m_texture);

        glBindTexture(m_textureTarget, m_texture);
        if (buf.isShm())
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        buf.bindToTexture();
    }

    buf.updateTexture();

    if (target)
        *target = m_textureTarget;

    return m_texture;
}

bool WindowCompositorView::isCursor() const
{
    return surface()->isCursorSurface();
}

void WindowCompositorView::onXdgSetMaximized()
{
    m_xdgSurface->requestMaximized(output()->geometry().size());

    // An improvement here, would have been to wait for the commit after the ack_configure for the
    // request above before moving the window. This would have prevented the window from being
    // moved until the contents of the window had actually updated. This improvement is left as an
    // exercise for the reader.
    setPosition(QPoint(0, 0));
}

void WindowCompositorView::onXdgUnsetMaximized()
{
    m_xdgSurface->requestUnMaximized();
}

void WindowCompositorView::onXdgSetFullscreen(QWaylandOutput* clientPreferredOutput)
{
    QWaylandOutput *outputToFullscreen = clientPreferredOutput
            ? clientPreferredOutput
            : output();

    m_xdgSurface->requestFullscreen(outputToFullscreen->geometry().size());

    // An improvement here, would have been to wait for the commit after the ack_configure for the
    // request above before moving the window. This would have prevented the window from being
    // moved until the contents of the window had actually updated. This improvement is left as an
    // exercise for the reader.
    setPosition(outputToFullscreen->position());
}

void WindowCompositorView::onOffsetForNextFrame(const QPoint &offset)
{
    m_offset = offset;
    setPosition(position() + offset);
}

void WindowCompositorView::onXdgUnsetFullscreen()
{
    onXdgUnsetMaximized();
}

WindowCompositor::WindowCompositor(QWindow *window)
    : QWaylandCompositor()
    , m_window(window)
    , m_wlShell(new QWaylandWlShell(this))
    , m_xdgShell(new QWaylandXdgShell(this))
{
    connect(m_wlShell, &QWaylandWlShell::shellSurfaceCreated, this, &WindowCompositor::onWlShellSurfaceCreated);
    connect(m_xdgShell, &QWaylandXdgShell::xdgSurfaceCreated, this, &WindowCompositor::onXdgSurfaceCreated);
    connect(m_xdgShell, &QWaylandXdgShell::createXdgPopup, this, &WindowCompositor::onCreateXdgPopup);
}

WindowCompositor::~WindowCompositor()
{
}

void WindowCompositor::create()
{
    new QWaylandOutput(this, m_window);
    QWaylandCompositor::create();

    connect(this, &QWaylandCompositor::surfaceCreated, this, &WindowCompositor::onSurfaceCreated);
    connect(defaultInputDevice(), &QWaylandInputDevice::cursorSurfaceRequest, this, &WindowCompositor::adjustCursorSurface);
    connect(defaultInputDevice()->drag(), &QWaylandDrag::dragStarted, this, &WindowCompositor::startDrag);

    connect(this, &QWaylandCompositor::subsurfaceChanged, this, &WindowCompositor::onSubsurfaceChanged);
}

void WindowCompositor::onSurfaceCreated(QWaylandSurface *surface)
{
    connect(surface, &QWaylandSurface::surfaceDestroyed, this, &WindowCompositor::surfaceDestroyed);
    connect(surface, &QWaylandSurface::mappedChanged, this, &WindowCompositor::surfaceMappedChanged);
    connect(surface, &QWaylandSurface::redraw, this, &WindowCompositor::triggerRender);

    connect(surface, &QWaylandSurface::subsurfacePositionChanged, this, &WindowCompositor::onSubsurfacePositionChanged);

    WindowCompositorView *view = new WindowCompositorView;
    view->setSurface(surface);
    view->setOutput(outputFor(m_window));
    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &WindowCompositor::viewSurfaceDestroyed);
    connect(surface, &QWaylandSurface::offsetForNextFrame, view, &WindowCompositorView::onOffsetForNextFrame);
}

void WindowCompositor::surfaceMappedChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    if (surface->isMapped()) {
        if (surface->role() == QWaylandWlShellSurface::role()
                || surface->role() == QWaylandXdgSurface::role()
                || surface->role() == QWaylandXdgPopup::role()) {
            defaultInputDevice()->setKeyboardFocus(surface);
        }
    } else if (popupActive()) {
        for (int i = 0; i < m_popupViews.count(); i++) {
            if (m_popupViews.at(i)->surface() == surface) {
                m_popupViews.removeAt(i);
                break;
            }
        }
    }
    triggerRender();
}

void WindowCompositor::surfaceDestroyed()
{
    triggerRender();
}

void WindowCompositor::viewSurfaceDestroyed()
{
    WindowCompositorView *view = qobject_cast<WindowCompositorView*>(sender());
    m_views.removeAll(view);
    delete view;
}

WindowCompositorView * WindowCompositor::findView(const QWaylandSurface *s) const
{
    Q_FOREACH (WindowCompositorView* view, m_views) {
        if (view->surface() == s)
            return view;
    }
    return Q_NULLPTR;
}

void WindowCompositor::onWlShellSurfaceCreated(QWaylandWlShellSurface *wlShellSurface)
{
    connect(wlShellSurface, &QWaylandWlShellSurface::startMove, this, &WindowCompositor::onStartMove);
    connect(wlShellSurface, &QWaylandWlShellSurface::startResize, this, &WindowCompositor::onWlStartResize);
    connect(wlShellSurface, &QWaylandWlShellSurface::setTransient, this, &WindowCompositor::onSetTransient);
    connect(wlShellSurface, &QWaylandWlShellSurface::setPopup, this, &WindowCompositor::onSetPopup);

    WindowCompositorView *view = findView(wlShellSurface->surface());
    Q_ASSERT(view);
    view->m_wlShellSurface = wlShellSurface;
}

void WindowCompositor::onXdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
{
    connect(xdgSurface, &QWaylandXdgSurface::startMove, this, &WindowCompositor::onStartMove);
    connect(xdgSurface, &QWaylandXdgSurface::startResize, this, &WindowCompositor::onXdgStartResize);

    WindowCompositorView *view = findView(xdgSurface->surface());
    Q_ASSERT(view);
    view->m_xdgSurface = xdgSurface;

    connect(xdgSurface, &QWaylandXdgSurface::setMaximized, view, &WindowCompositorView::onXdgSetMaximized);
    connect(xdgSurface, &QWaylandXdgSurface::setFullscreen, view, &WindowCompositorView::onXdgSetFullscreen);
    connect(xdgSurface, &QWaylandXdgSurface::unsetMaximized, view, &WindowCompositorView::onXdgUnsetMaximized);
    connect(xdgSurface, &QWaylandXdgSurface::unsetFullscreen, view, &WindowCompositorView::onXdgUnsetFullscreen);
}

void WindowCompositor::onCreateXdgPopup(QWaylandSurface *surface, QWaylandSurface *parent,
                                        QWaylandInputDevice *inputDevice, const QPoint &position,
                                        const QWaylandResource &resource)
{
    Q_UNUSED(inputDevice);

    QWaylandXdgPopup *xdgPopup = new QWaylandXdgPopup(m_xdgShell, surface, parent, resource);

    WindowCompositorView *view = findView(surface);
    Q_ASSERT(view);

    WindowCompositorView *parentView = findView(parent);
    Q_ASSERT(parentView);

    view->setPosition(parentView->position() + position);
    view->m_xdgPopup = xdgPopup;
}

void WindowCompositor::onStartMove()
{
    closePopups();
    emit startMove();
}

void WindowCompositor::onWlStartResize(QWaylandInputDevice *, QWaylandWlShellSurface::ResizeEdge edges)
{
    closePopups();
    emit startResize(int(edges), false);
}

void WindowCompositor::onXdgStartResize(QWaylandInputDevice *inputDevice,
                                        QWaylandXdgSurface::ResizeEdge edges)
{
    Q_UNUSED(inputDevice);
    emit startResize(int(edges), true);
}

void WindowCompositor::onSetTransient(QWaylandSurface *parent, const QPoint &relativeToParent, QWaylandWlShellSurface::FocusPolicy focusPolicy)
{
    QWaylandWlShellSurface *wlShellSurface = qobject_cast<QWaylandWlShellSurface*>(sender());
    WindowCompositorView *view = findView(wlShellSurface->surface());

    if (view) {
        raise(view);
        WindowCompositorView *parentView = findView(parent);
        if (parentView)
            view->setPosition(parentView->position() + relativeToParent);
    }
}

void WindowCompositor::onSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(inputDevice);
    QWaylandWlShellSurface *surface = qobject_cast<QWaylandWlShellSurface*>(sender());
    WindowCompositorView *view = findView(surface->surface());
    m_popupViews << view;
    if (view) {
        raise(view);
        WindowCompositorView *parentView = findView(parent);
        if (parentView)
            view->setPosition(parentView->position() + relativeToParent);
    }
}

void WindowCompositor::onSubsurfaceChanged(QWaylandSurface *child, QWaylandSurface *parent)
{
    WindowCompositorView *view = findView(child);
    WindowCompositorView *parentView = findView(parent);
    view->setParentView(parentView);
}

void WindowCompositor::onSubsurfacePositionChanged(const QPoint &position)
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface*>(sender());
    if (!surface)
        return;
    WindowCompositorView *view = findView(surface);
    view->setPosition(position);
    triggerRender();
}

void WindowCompositor::triggerRender()
{
    m_window->requestUpdate();
}

void WindowCompositor::startRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->frameStarted();
}

void WindowCompositor::endRender()
{
    QWaylandOutput *out = defaultOutput();
    if (out)
        out->sendFrameCallbacks();
}

void WindowCompositor::updateCursor()
{
    m_cursorView.advance();
    QImage image = m_cursorView.currentBuffer().image();
    if (!image.isNull())
        m_window->setCursor(QCursor(QPixmap::fromImage(image), m_cursorHotspotX, m_cursorHotspotY));
}

void WindowCompositor::adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    if ((m_cursorView.surface() != surface)) {
        if (m_cursorView.surface())
            disconnect(m_cursorView.surface(), &QWaylandSurface::redraw, this, &WindowCompositor::updateCursor);
        if (surface)
            connect(surface, &QWaylandSurface::redraw, this, &WindowCompositor::updateCursor);
    }

    m_cursorView.setSurface(surface);
    m_cursorHotspotX = hotspotX;
    m_cursorHotspotY = hotspotY;

    if (surface && surface->isMapped())
        updateCursor();
}

void WindowCompositor::closePopups()
{
    Q_FOREACH (WindowCompositorView *view, m_popupViews) {
        if (view->m_wlShellSurface)
            view->m_wlShellSurface->sendPopupDone();
    }
    m_popupViews.clear();

    m_xdgShell->closeAllPopups();
}

void WindowCompositor::handleMouseEvent(QWaylandView *target, QMouseEvent *me)
{
    if (target && popupActive() && me->type() == QEvent::MouseButtonPress
        && target->surface()->client() != m_popupViews.first()->surface()->client()) {
        closePopups();
    }
    QWaylandInputDevice *input = defaultInputDevice();
    QWaylandSurface *surface = target ? target->surface() : nullptr;
    switch (me->type()) {
        case QEvent::MouseButtonPress:
            input->sendMousePressEvent(me->button());
            if (surface != input->keyboardFocus()) {
                if (surface == nullptr
                        || surface->role() == QWaylandWlShellSurface::role()
                        || surface->role() == QWaylandXdgSurface::role()
                        || surface->role() == QWaylandXdgPopup::role()) {
                    input->setKeyboardFocus(surface);
                }
            }
            break;
    case QEvent::MouseButtonRelease:
         input->sendMouseReleaseEvent(me->button());
         break;
    case QEvent::MouseMove:
        input->sendMouseMoveEvent(target, me->localPos(), me->globalPos());
    default:
        break;
    }
}

void WindowCompositor::handleResize(WindowCompositorView *target, const QSize &initialSize, const QPoint &delta, int edge)
{
    QWaylandWlShellSurface *wlShellSurface = target->m_wlShellSurface;
    if (wlShellSurface) {
        QWaylandWlShellSurface::ResizeEdge edges = QWaylandWlShellSurface::ResizeEdge(edge);
        QSize newSize = wlShellSurface->sizeForResize(initialSize, delta, edges);
        wlShellSurface->sendConfigure(newSize, edges);
    }

    QWaylandXdgSurface *xdgSurface = target->m_xdgSurface;
    if (xdgSurface) {
        QWaylandXdgSurface::ResizeEdge edges = static_cast<QWaylandXdgSurface::ResizeEdge>(edge);
        QSize newSize = xdgSurface->sizeForResize(initialSize, delta, edges);
        xdgSurface->requestResizing(newSize);
    }
}

void WindowCompositor::startDrag()
{
    QWaylandDrag *currentDrag = defaultInputDevice()->drag();
    Q_ASSERT(currentDrag);
    WindowCompositorView *iconView = findView(currentDrag->icon());
    iconView->setPosition(m_window->mapFromGlobal(QCursor::pos()));

    emit dragStarted(iconView);
}

void WindowCompositor::handleDrag(WindowCompositorView *target, QMouseEvent *me)
{
    QPointF pos = me->localPos();
    QWaylandSurface *surface = 0;
    if (target) {
        pos -= target->position();
        surface = target->surface();
    }
    QWaylandDrag *currentDrag = defaultInputDevice()->drag();
    currentDrag->dragMove(surface, pos);
    if (me->buttons() == Qt::NoButton)
        currentDrag->drop();
}

// We only have a flat list of views, plus pointers from child to parent,
// so maintaining a stacking order gets a bit complex. A better data
// structure is left as an exercise for the reader.

static int findEndOfChildTree(const QList<WindowCompositorView*> &list, int index)
{
    int n = list.count();
    WindowCompositorView *parent = list.at(index);
    while (index + 1 < n) {
        if (list.at(index+1)->parentView() != parent)
            break;
        index = findEndOfChildTree(list, index + 1);
    }
    return index;
}

void WindowCompositor::raise(WindowCompositorView *view)
{
    int startPos = m_views.indexOf(view);
    int endPos = findEndOfChildTree(m_views, startPos);

    int n = m_views.count();
    int tail =  n - endPos - 1;

    //bubble sort: move the child tree to the end of the list
    for (int i = 0; i < tail; i++) {
        int source = endPos + 1 + i;
        int dest = startPos + i;
        for (int j = source; j > dest; j--)
            m_views.swap(j, j-1);
    }
}
