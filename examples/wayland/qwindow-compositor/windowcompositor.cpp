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

#include <QtWaylandCompositor/QWaylandOutputSpace>
#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QtWaylandCompositor/qwaylandinput.h>

#include <QDebug>

GLuint WindowCompositorView::getTexture() {
    if (advance()) {
        if (m_texture)
            glDeleteTextures(1, &m_texture);

        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        currentBuffer().bindToTexture();
    }
    return m_texture;
}

WindowCompositor::WindowCompositor(QWindow *window)
    : QWaylandCompositor()
    , m_window(window)
    , m_shell(new QWaylandShell(this))
{
    connect(m_shell, &QWaylandShell::createShellSurface, this, &WindowCompositor::onCreateShellSurface);
}

WindowCompositor::~WindowCompositor()
{
}

void WindowCompositor::create()
{
    QWaylandCompositor::create();
    new QWaylandOutput(defaultOutputSpace(), m_window);

    connect(this, &QWaylandCompositor::surfaceCreated, this, &WindowCompositor::onSurfaceCreated);
    connect(defaultInputDevice(), &QWaylandInputDevice::cursorSurfaceRequest, this, &WindowCompositor::adjustCursorSurface);
}

void WindowCompositor::onSurfaceCreated(QWaylandSurface *surface)
{
    qDebug() << "onSurfaceCreated" << surface;

    connect(surface, &QWaylandSurface::surfaceDestroyed, this, &WindowCompositor::surfaceDestroyed);
    connect(surface, &QWaylandSurface::mappedChanged, this, &WindowCompositor::surfaceMappedChanged);
    connect(surface, &QWaylandSurface::redraw, this, &WindowCompositor::triggerRender);
}

void WindowCompositor::surfaceMappedChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    qDebug() << "surfaceMappedChanged()" << surface << surface->isMapped();
    if (surface->isMapped()) {\
        if (!surface->isCursorSurface())
        defaultInputDevice()->setKeyboardFocus(surface);
    }
    triggerRender();
}

void WindowCompositor::surfaceDestroyed()
{
    // QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
     qDebug() << "surfaceDestroyed()";
    // Q_FOREACH (QWaylandView *view, surface->views()) {
    //     int n = m_views.removeAll(static_cast<WindowCompositorView*>(view));
    //     qDebug() << n << view;
    // }
    triggerRender();
}

void WindowCompositor::viewSurfaceDestroyed()
{
    WindowCompositorView *view = qobject_cast<WindowCompositorView*>(sender());
    int n = m_views.removeAll(view);
    qDebug() << n << view;
    delete view;
}

void WindowCompositor::surfaceCommittedSlot()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

//    qDebug() << "surfaceCommittedSlot()" << surface;
    triggerRender();
}

void WindowCompositor::onCreateShellSurface(QWaylandSurface *s, QWaylandClient *client, uint id)
{
    qDebug() << "onCreateShellSurface" << s << client << id;
    QWaylandSurface *surface = s;

    WindowCompositorView *view = new WindowCompositorView;
    view->setSurface(surface);
    view->setOutput(output(m_window));
    m_views << view;
    connect(view, &QWaylandView::surfaceDestroyed, this, &WindowCompositor::viewSurfaceDestroyed);
    connect(view, &QWaylandView::requestedPositionChanged, this, &WindowCompositor::triggerRender);

    QWaylandShellSurface *shellSurface = new QWaylandShellSurface(m_shell, surface, view, client, id);
}

void WindowCompositor::triggerRender()
{
    m_window->requestUpdate();
}

void WindowCompositor::startRender()
{
    defaultOutput()->frameStarted();
    cleanupGraphicsResources();
}

void WindowCompositor::endRender()
{
    defaultOutput()->sendFrameCallbacks();
}

void WindowCompositor::adjustCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    //qDebug() << "adjustCursorSurface" << surface << hotspotY << hotspotY;
}

void WindowCompositor::handleMouseEvent(QWaylandView *target, QMouseEvent *me)
{
    QWaylandInputDevice *input = defaultInputDevice();
    switch (me->type()) {
        case QEvent::MouseButtonPress:
            input->sendMousePressEvent(me->button());
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
