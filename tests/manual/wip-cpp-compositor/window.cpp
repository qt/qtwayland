// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "window.h"
#include "compositor.h"

#include <QPainter>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMouseEvent>

Window::Window(Compositor *compositor)
    : m_compositor(compositor)
    , m_output(new QWaylandOutput(compositor, this))
{
    m_output->setSizeFollowsWindow(true);
}

void Window::initializeGL()
{
    m_textureBlitter.create();
    m_compositor->handleGlInitialized();
}

void Window::paintGL()
{
    m_output->frameStarted();

    QOpenGLFunctions *functions = context()->functions();
    functions->glClearColor(.4f, .7f, .1f, 0.5f);
    functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLenum currentTarget = GL_TEXTURE_2D;
    m_textureBlitter.bind(currentTarget);
    functions->glEnable(GL_BLEND);
    functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto views = m_compositor->views();

    for (View *view : views) {
        auto *texture = view->getTexture();
        if (!texture)
            continue;
        if (texture->target() != currentTarget) {
            currentTarget = texture->target();
            m_textureBlitter.bind(currentTarget);
        }
        GLuint textureId = texture->textureId();

        // Anchored position needs to be updated immediately before rendering or
        // position and size might get out of sync.
        view->updateAnchoredPosition();

        QWaylandSurface *surface = view->surface();
        if (surface && surface->hasContent()) {
            QOpenGLTextureBlitter::Origin surfaceOrigin =
                    view->currentBuffer().origin() == QWaylandSurface::OriginTopLeft
                    ? QOpenGLTextureBlitter::OriginTopLeft
                    : QOpenGLTextureBlitter::OriginBottomLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(view->globalGeometry(), QRect(QPoint(), size()));
            m_textureBlitter.blit(textureId, targetTransform, surfaceOrigin);
        }
    }
    m_textureBlitter.release();

    m_output->sendFrameCallbacks();
}

void Window::mousePressEvent(QMouseEvent *event)
{
    m_compositor->handleMousePress(event->position().toPoint(), event->button());
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    m_compositor->handleMouseRelease(event->position().toPoint(), event->button(), event->buttons());
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    m_compositor->handleMouseMove(event->position().toPoint());
}

#if QT_CONFIG(wheelevent)
void Window::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().x() != 0)
        m_compositor->handleMouseWheel(Qt::Horizontal, event->angleDelta().x());
    if (event->angleDelta().y() != 0)
        m_compositor->handleMouseWheel(Qt::Vertical, event->angleDelta().y());
}
#endif

void Window::keyPressEvent(QKeyEvent *event)
{
    m_compositor->handleKeyPress(event->nativeScanCode());
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    m_compositor->handleKeyRelease(event->nativeScanCode());
}
