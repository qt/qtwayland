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

#include "compositorwindow.h"

#include <QMouseEvent>
#include <QOpenGLWindow>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

#include "windowcompositor.h"
#include <QtWaylandCompositor/qwaylandinput.h>

CompositorWindow::CompositorWindow()
    : m_backgroundTexture(0)
    , m_compositor(0)
    , m_grabState(NoGrab)
    , m_dragIconView(0)
{
}

void CompositorWindow::setCompositor(WindowCompositor *comp) {
    m_compositor = comp;
    connect(m_compositor, &WindowCompositor::startMove, this, &CompositorWindow::startMove);
    connect(m_compositor, &WindowCompositor::startResize, this, &CompositorWindow::startResize);
    connect(m_compositor, &WindowCompositor::dragStarted, this, &CompositorWindow::startDrag);
}

void CompositorWindow::initializeGL()
{
    QImage backgroundImage = QImage(QLatin1String(":/background.jpg"));
    m_backgroundTexture = new QOpenGLTexture(backgroundImage, QOpenGLTexture::DontGenerateMipMaps);
    m_backgroundTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_backgroundImageSize = backgroundImage.size();
    m_textureBlitter.create();
}

void CompositorWindow::drawBackground()
{
    for (int y = 0; y < height(); y += m_backgroundImageSize.height()) {
        for (int x = 0; x < width(); x += m_backgroundImageSize.width()) {
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(QRect(QPoint(x,y), m_backgroundImageSize), QRect(QPoint(0,0), size()));
            m_textureBlitter.blit(m_backgroundTexture->textureId(),
                              targetTransform,
                              QOpenGLTextureBlitter::OriginTopLeft);
        }
    }
}

QPointF CompositorWindow::getAnchorPosition(const QPointF &position, int resizeEdge, const QSize &windowSize)
{
    float y = position.y();
    if (resizeEdge & QWaylandXdgSurface::ResizeEdge::TopEdge)
        y += windowSize.height();

    float x = position.x();
    if (resizeEdge & QWaylandXdgSurface::ResizeEdge::LeftEdge)
        x += windowSize.width();

    return QPointF(x, y);
}

QPointF CompositorWindow::getAnchoredPosition(const QPointF &anchorPosition, int resizeEdge, const QSize &windowSize)
{
    return anchorPosition - getAnchorPosition(QPointF(), resizeEdge, windowSize);
}

void CompositorWindow::paintGL()
{
    m_compositor->startRender();
    QOpenGLFunctions *functions = context()->functions();
    functions->glClearColor(1.f, .6f, .0f, 0.5f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_textureBlitter.bind();
    drawBackground();

    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLenum currentTarget = GL_TEXTURE_2D;
    Q_FOREACH (WindowCompositorView *view, m_compositor->views()) {
        if (view->isCursor())
            continue;
        GLenum target;
        GLuint textureId = view->getTexture(&target);
        if (!textureId || !target)
            continue;
        if (target != currentTarget) {
            currentTarget = target;
            m_textureBlitter.bind(currentTarget);
        }
        QWaylandSurface *surface = view->surface();
        if (surface && surface->isMapped()) {
            QSize s = surface->size();
            if (!s.isEmpty()) {
                if (m_mouseView == view && m_grabState == ResizeGrab && m_resizeAnchored)
                    view->setPosition(getAnchoredPosition(m_resizeAnchorPosition, m_resizeEdge, s));
                QPointF pos = view->position() + view->parentPosition();
                QRectF surfaceGeometry(pos, s);
                QOpenGLTextureBlitter::Origin surfaceOrigin =
                    view->currentBuffer().origin() == QWaylandSurface::OriginTopLeft
                    ? QOpenGLTextureBlitter::OriginTopLeft
                    : QOpenGLTextureBlitter::OriginBottomLeft;
                QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(surfaceGeometry, QRect(QPoint(), size()));
                m_textureBlitter.blit(textureId, targetTransform, surfaceOrigin);
            }
        }
    }
    functions->glDisable(GL_BLEND);

    m_textureBlitter.release();
    m_compositor->endRender();
}

WindowCompositorView *CompositorWindow::viewAt(const QPointF &point)
{
    WindowCompositorView *ret = 0;
    Q_FOREACH (WindowCompositorView *view, m_compositor->views()) {
        if (view == m_dragIconView)
            continue;
        QPointF topLeft = view->position();
        QWaylandSurface *surface = view->surface();
        QRectF geo(topLeft, surface->size());
        if (geo.contains(point))
            ret = view;
    }
    return ret;
}

void CompositorWindow::startMove()
{
    m_grabState = MoveGrab;
}

void CompositorWindow::startResize(int edge, bool anchored)
{
    m_initialSize = m_mouseView->windowSize();
    m_grabState = ResizeGrab;
    m_resizeEdge = edge;
    m_resizeAnchored = anchored;
    m_resizeAnchorPosition = getAnchorPosition(m_mouseView->position(), edge, m_mouseView->surface()->size());
}

void CompositorWindow::startDrag(WindowCompositorView *dragIcon)
{
    m_grabState = DragGrab;
    m_dragIconView = dragIcon;
    m_compositor->raise(dragIcon);
}

void CompositorWindow::mousePressEvent(QMouseEvent *e)
{
    if (mouseGrab())
        return;
    if (m_mouseView.isNull()) {
        m_mouseView = viewAt(e->localPos());
        if (!m_mouseView) {
            m_compositor->closePopups();
            return;
        }
        if (e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::MetaModifier)
            m_grabState = MoveGrab; //start move
        else
            m_compositor->raise(m_mouseView);
        m_initialMousePos = e->localPos();
        m_mouseOffset = e->localPos() - m_mouseView->position();

        QMouseEvent moveEvent(QEvent::MouseMove, e->localPos(), e->globalPos(), Qt::NoButton, Qt::NoButton, e->modifiers());
        sendMouseEvent(&moveEvent, m_mouseView);
    }
    sendMouseEvent(e, m_mouseView);
}

void CompositorWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (!mouseGrab())
        sendMouseEvent(e, m_mouseView);
    if (e->buttons() == Qt::NoButton) {
        if (m_grabState == DragGrab) {
            WindowCompositorView *view = viewAt(e->localPos());
            m_compositor->handleDrag(view, e);
        }
        m_mouseView = 0;
        m_grabState = NoGrab;
    }
}

void CompositorWindow::mouseMoveEvent(QMouseEvent *e)
{
    switch (m_grabState) {
    case NoGrab: {
        WindowCompositorView *view = m_mouseView ? m_mouseView.data() : viewAt(e->localPos());
        sendMouseEvent(e, view);
        if (!view)
            setCursor(Qt::ArrowCursor);
    }
        break;
    case MoveGrab: {
        m_mouseView->setPosition(e->localPos() - m_mouseOffset);
        update();
    }
        break;
    case ResizeGrab: {
        QPoint delta = (e->localPos() - m_initialMousePos).toPoint();
        m_compositor->handleResize(m_mouseView, m_initialSize, delta, m_resizeEdge);
    }
        break;
    case DragGrab: {
        WindowCompositorView *view = viewAt(e->localPos());
        m_compositor->handleDrag(view, e);
        if (m_dragIconView) {
            m_dragIconView->setPosition(e->localPos() + m_dragIconView->offset());
            update();
        }
    }
        break;
    }
}

void CompositorWindow::sendMouseEvent(QMouseEvent *e, WindowCompositorView *target)
{
    if (!target)
        return;

    QPointF mappedPos = e->localPos() - target->position();
    QMouseEvent viewEvent(e->type(), mappedPos, e->localPos(), e->button(), e->buttons(), e->modifiers());
    m_compositor->handleMouseEvent(target, &viewEvent);
}

void CompositorWindow::keyPressEvent(QKeyEvent *e)
{
    m_compositor->defaultInputDevice()->sendKeyPressEvent(e->nativeScanCode());
}

void CompositorWindow::keyReleaseEvent(QKeyEvent *e)
{
    m_compositor->defaultInputDevice()->sendKeyReleaseEvent(e->nativeScanCode());
}
