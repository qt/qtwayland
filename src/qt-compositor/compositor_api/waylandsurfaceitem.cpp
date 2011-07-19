/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "waylandsurfaceitem.h"
#include "waylandsurface.h"

#include <QtDeclarative/QSGEngine>

#include <QtGui/QKeyEvent>

#include <QtDeclarative/QSGSimpleTextureNode>
#include <QtDeclarative/QSGSimpleRectNode>
#include <QtDeclarative/QSGCanvas>

void WaylandSurfaceItem::surfaceDamaged(const QRect &)
{
    QSGTexture *oldTexture = m_texture;

    if (m_surface->type() == WaylandSurface::Texture) {
        QSGEngine::TextureOption opt = useTextureAlpha() ? QSGEngine::TextureHasAlphaChannel : QSGEngine::TextureOption(0);

        m_texture = canvas()->sceneGraphEngine()->createTextureFromId(m_surface->texture(),
                                                                      m_surface->geometry().size(),
                                                                      opt);
    } else {
        m_texture = canvas()->sceneGraphEngine()->createTextureFromImage(m_surface->image());
    }

    delete oldTexture;

    emit textureChanged();
}

WaylandSurfaceItem::WaylandSurfaceItem(QSGItem *parent)
    : QSGItem(parent)
    , m_surface(0)
    , m_texture(0)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
{
}

WaylandSurfaceItem::WaylandSurfaceItem(WaylandSurface *surface, QSGItem *parent)
    : QSGItem(parent)
    , m_surface(0)
    , m_texture(0)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
{
    init(surface);
}

void WaylandSurfaceItem::init(WaylandSurface *surface)
{
    if (!surface)
        return;

    m_surface = surface;

    setWidth(surface->geometry().width());
    setHeight(surface->geometry().height());

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    connect(surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(this, SIGNAL(textureChanged()), this, SLOT(update()));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));

}

WaylandSurfaceItem::~WaylandSurfaceItem()
{
    delete m_texture;
}

void WaylandSurfaceItem::setSurface(WaylandSurface *surface)
{
    init(surface);
}

bool WaylandSurfaceItem::isYInverted() const
{
    return m_surface->isYInverted();
}

QSGTexture *WaylandSurfaceItem::texture() const
{
    if (m_texture)
        m_texture->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    return m_texture;
}

void WaylandSurfaceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_surface)
        m_surface->sendMousePressEvent(toSurface(event->pos()), event->button());
}

void WaylandSurfaceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_surface)
        m_surface->sendMouseMoveEvent(toSurface(event->pos()));
}

void WaylandSurfaceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_surface)
        m_surface->sendMouseReleaseEvent(toSurface(event->pos()), event->button());
}

void WaylandSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus())
        m_surface->sendKeyPressEvent(event->nativeScanCode());
}

void WaylandSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus())
        m_surface->sendKeyReleaseEvent(event->nativeScanCode());
}

void WaylandSurfaceItem::touchEvent(QTouchEvent *event)
{
    if (m_touchEventsEnabled && m_surface && hasFocus()) {
        event->accept();
        QList<QTouchEvent::TouchPoint> points = event->touchPoints();
        if (!points.isEmpty()) {
            for (int i = 0; i < points.count(); ++i) {
                const QTouchEvent::TouchPoint &point(points.at(i));
                // Wayland expects surface-relative coordinates.
                m_surface->sendTouchPointEvent(point.id(), point.pos().x(), point.pos().y(), point.state());
            }
            m_surface->sendTouchFrameEvent();
        }
    } else {
        event->ignore();
    }
}

void WaylandSurfaceItem::takeFocus()
{
    setFocus(true);

    if (m_surface)
        m_surface->setInputFocus();
}

QPoint WaylandSurfaceItem::toSurface(const QPointF &pos) const
{
    return pos.toPoint();
}

void WaylandSurfaceItem::surfaceMapped(const QSize &size)
{
    setWidth(size.width());
    setHeight(size.height());
}

void WaylandSurfaceItem::surfaceDestroyed(QObject *)
{
    m_surface = 0;
}

bool WaylandSurfaceItem::paintEnabled() const
{
    return m_paintEnabled;
}

void WaylandSurfaceItem::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

QSGNode *WaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!m_texture || !m_paintEnabled) {
        delete oldNode;
        return 0;
    }

    if (!node) {
        node = new QSGSimpleTextureNode();
    }

    if (surface()->isYInverted()) {
        node->setRect(0, height(), width(), -height());
    } else {
        node->setRect(0, 0, width(), height());
    }

    node->setTexture(m_texture);
    node->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);

    return node;
}

void WaylandSurfaceItem::setUseTextureAlpha(bool useTextureAlpha)
{
    m_useTextureAlpha = useTextureAlpha;

    if ((flags() & ItemHasContents) != 0) {
        update();
    }
}

void WaylandSurfaceItem::setClientRenderingEnabled(bool enabled)
{
    if (m_clientRenderingEnabled != enabled) {
        m_clientRenderingEnabled = enabled;

        if (m_surface) {
            m_surface->sendOnScreenVisibilityChange(enabled);
        }

        emit clientRenderingEnabledChanged();       
    }
}

// By default touch events are ignored and not sent to the clients.
//
// Call this function to enable it, however this approach is INEFFICIENT.
//
// The touch support in WaylandSurfaceItem is only there to support
// experimenting and to simplify the life of "dummy" compositors.
//
// Ideally touch events should be sent to the client right when they are read
// from the hardware, there is no need to waste time with sending them through
// Qt. (or, if the compositor contains UI too, both the Wayland and the Qt touch
// events should be sent at the same time, otherwise the clients will inevitably
// suffer from a small lag which may get problematic on slower systems with a
// large number of touch events)
//
void WaylandSurfaceItem::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}
