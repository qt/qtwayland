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

#include <qsgengine.h>
#include <private/qsgitem_p.h>

#include <QKeyEvent>

#include <qsgsimpletexturenode.h>
#include <qsgsimplerectnode.h>

void WaylandSurfaceItem::surfaceDamaged(const QRect &)
{
    if (m_texture)
        delete m_texture;

    if (m_surface->type() == WaylandSurface::Texture) {
        m_texture = canvas()->sceneGraphEngine()->createTextureFromId(m_surface->texture(),
                                                                      m_surface->geometry().size());
    } else {
        m_texture = canvas()->sceneGraphEngine()->createTextureFromImage(m_surface->image());
    }

    emit textureChanged();
}

WaylandSurfaceItem::WaylandSurfaceItem(QSGItem *parent)
    : QSGItem(parent)
    , m_surface(0)
    , m_texture(0)
{
}

WaylandSurfaceItem::WaylandSurfaceItem(WaylandSurface *surface, QSGItem *parent)
    : QSGItem(parent)
    , m_surface(0)
    , m_texture(0)
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
    connect(surface, SIGNAL(mapped(const QRect &)), this, SLOT(surfaceMapped(const QRect &)));
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

void WaylandSurfaceItem::surfaceMapped(const QRect &rect)
{
    setWidth(rect.width());
    setHeight(rect.height());
}

void WaylandSurfaceItem::surfaceDestroyed(QObject *)
{
    m_surface = 0;
}

QSGNode *WaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!m_texture) {
        delete oldNode;
        return 0;
    }

    if (!node) {
        node = new QSGSimpleTextureNode();
        node->setTexture(m_texture);
    }

    node->setRect(QRectF(0, height(), width(), -height()));
    node->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);

    return node;
}
