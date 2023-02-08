// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "window.h"
#include "compositor.h"

#include <QPainter>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMouseEvent>

Window::Window()
{
}

void Window::setCompositor(Compositor *comp) {
    m_compositor = comp;
}

void Window::initializeGL()
{
    m_textureBlitter.create();
    emit glReady();
}

//! [paintGL]
void Window::paintGL()
{
    m_compositor->startRender();

    QOpenGLFunctions *functions = context()->functions();
    functions->glClearColor(.4f, .7f, .1f, 0.5f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLenum currentTarget = GL_TEXTURE_2D;
    m_textureBlitter.bind(currentTarget);
    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto views = m_compositor->views();
    for (View *view : views) {
        auto texture = view->getTexture();
        if (!texture)
            continue;
        if (texture->target() != currentTarget) {
            currentTarget = texture->target();
            m_textureBlitter.bind(currentTarget);
        }
        GLuint textureId = texture->textureId();
        QWaylandSurface *surface = view->surface();
        if (surface && surface->hasContent()) {
            QSize s = surface->destinationSize();
            view->initPosition(size(), s);
            QPointF pos = view->globalPosition();
            QRectF surfaceGeometry(pos, s);
            QOpenGLTextureBlitter::Origin surfaceOrigin =
                    view->currentBuffer().origin() == QWaylandSurface::OriginTopLeft
                    ? QOpenGLTextureBlitter::OriginTopLeft
                    : QOpenGLTextureBlitter::OriginBottomLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(surfaceGeometry, QRect(QPoint(), size()));
            m_textureBlitter.blit(textureId, targetTransform, surfaceOrigin);
        }
    }
    m_textureBlitter.release();
    m_compositor->endRender();
}
//! [paintGL]

//! [mousePressEvent]
void Window::mousePressEvent(QMouseEvent *event)
{
    m_compositor->handleMousePress(event->position().toPoint(), event->button());
}
//! [mousePressEvent]

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    m_compositor->handleMouseRelease(event->position().toPoint(), event->button(), event->buttons());
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    m_compositor->handleMouseMove(event->position().toPoint());
}

void Window::wheelEvent(QWheelEvent *event)
{
    m_compositor->handleMouseWheel(event->angleDelta());
}

//! [keyPressEvent]
void Window::keyPressEvent(QKeyEvent *e)
{
    m_compositor->handleKeyPress(e->nativeScanCode());
}
//! [keyPressEvent]

void Window::keyReleaseEvent(QKeyEvent *e)
{
    m_compositor->handleKeyRelease(e->nativeScanCode());
}
