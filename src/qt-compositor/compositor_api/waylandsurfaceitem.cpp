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

#include <private/qsgadaptationlayer_p.h>
#include <private/qsgcontext_p.h>
#include <private/qsgitem_p.h>
#include <private/qsgtexture_p.h>
#include <private/qsgrectangle_p.h>
#include <private/qsgtextureprovider_p.h>

#include <QKeyEvent>

class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    WaylandSurfaceTextureProvider(WaylandSurface *surface);
    ~WaylandSurfaceTextureProvider();

    QSGTextureRef texture() {
        return m_textureRef;
    }

private slots:
    void surfaceDamaged(const QRect &rect);

private:
    WaylandSurface *m_surface;

    QSGPlainTexture *m_texture;
    QSGTextureRef m_textureRef;
};

WaylandSurfaceTextureProvider::WaylandSurfaceTextureProvider(WaylandSurface *surface)
    : m_surface(surface)
    , m_texture(new QSGPlainTexture)
    , m_textureRef(m_texture)
{
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
}

WaylandSurfaceTextureProvider::~WaylandSurfaceTextureProvider()
{
}

void WaylandSurfaceTextureProvider::surfaceDamaged(const QRect &)
{
    if (m_surface->type() == WaylandSurface::Texture) {
        m_texture->setTextureId(m_surface->texture());
        m_texture->setHasAlphaChannel(false);
        m_texture->setTextureSize(m_surface->geometry().size());
    } else {
        m_texture->setImage(m_surface->image());
    }

    m_texture->setOwnsTexture(true);
    m_texture->setHasMipmaps(false);

    emit textureChanged();
}

WaylandSurfaceItem::WaylandSurfaceItem(WaylandSurface *surface, QSGItem *parent)
    : QSGItem(parent)
    , m_surface(surface)
    , m_textureProvider(new WaylandSurfaceTextureProvider(surface))
{
    setWidth(surface->geometry().width());
    setHeight(surface->geometry().height());

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    connect(surface, SIGNAL(mapped(const QRect &)), this, SLOT(surfaceMapped(const QRect &)));
    connect(m_textureProvider, SIGNAL(textureChanged()), this, SLOT(update()));
}

WaylandSurfaceItem::~WaylandSurfaceItem()
{
    delete m_textureProvider;
}


QSGTextureProvider *WaylandSurfaceItem::textureProvider() const
{
    return m_textureProvider;
}

void WaylandSurfaceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMousePressEvent(toSurface(event->pos()), event->button());
}

void WaylandSurfaceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMouseMoveEvent(toSurface(event->pos()));
}

void WaylandSurfaceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMouseReleaseEvent(toSurface(event->pos()), event->button());
}

void WaylandSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (hasFocus())
        m_surface->sendKeyPressEvent(event->nativeScanCode());
}

void WaylandSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (hasFocus())
        m_surface->sendKeyReleaseEvent(event->nativeScanCode());
}

void WaylandSurfaceItem::takeFocus()
{
    setFocus(true);
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

    update();
}

QSGNode *WaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGImageNode *node = static_cast<QSGImageNode *>(oldNode);
    if (!node) {
        node = QSGContext::current->createImageNode();
        node->setFlag(QSGNode::UsePreprocess, false);
        node->setTexture(m_textureProvider);
    }

    m_textureProvider->setHorizontalWrapMode(QSGTextureProvider::ClampToEdge);
    m_textureProvider->setVerticalWrapMode(QSGTextureProvider::ClampToEdge);
    m_textureProvider->setFiltering(QSGItemPrivate::get(this)->smooth
                                    ? QSGTextureProvider::Linear : QSGTextureProvider::Nearest);

    node->setTargetRect(QRectF(0, 0, width(), height()));
    node->setSourceRect(QRectF(0, 0, 1, 1));
    node->update();

    return node;
}

#include "waylandsurfaceitem.moc"
