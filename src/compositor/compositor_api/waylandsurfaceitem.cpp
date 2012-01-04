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

#include "wlsurface.h"
#include "wlextendedsurface.h"

#include <QtGui/QKeyEvent>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QSGSimpleRectNode>
#include <QtQuick/QQuickCanvas>

class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    WaylandSurfaceTextureProvider() : t(0) { }

    QSGTexture *texture() const {
        if (t)
            t->setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return t;
    }

    QSGTexture *t;
    bool smooth;
};

WaylandSurfaceItem::WaylandSurfaceItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_texture(0)
    , m_provider(0)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
{
}

WaylandSurfaceItem::WaylandSurfaceItem(WaylandSurface *surface, QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_texture(0)
    , m_provider(0)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
{
    init(surface);
}

void WaylandSurfaceItem::init(WaylandSurface *surface)
{
    if (!surface)
        return;

    if (m_surface) {
        m_surface->setSurfaceItem(0);
    }

    m_surface = surface;
    m_surface->setSurfaceItem(this);

    setWidth(surface->geometry().width());
    setHeight(surface->geometry().height());

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    connect(surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    connect(surface, SIGNAL(parentChanged(WaylandSurface*,WaylandSurface*)),
            this, SLOT(parentChanged(WaylandSurface*,WaylandSurface*)));
    connect(surface, SIGNAL(geometryChanged()), this, SLOT(updateGeometry()));

    m_damaged = false;

}

WaylandSurfaceItem::~WaylandSurfaceItem()
{
    if (m_surface) {
        m_surface->setSurfaceItem(0);
    }
    m_texture->deleteLater();
}

void WaylandSurfaceItem::setSurface(WaylandSurface *surface)
{
    init(surface);
}

bool WaylandSurfaceItem::isYInverted() const
{
    return m_surface->isYInverted();
}

QSGTextureProvider *WaylandSurfaceItem::textureProvider() const
{
    if (!m_provider)
        m_provider = new WaylandSurfaceTextureProvider();
    return m_provider;
}

void WaylandSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (m_surface)
        m_surface->sendMousePressEvent(toSurface(event->pos()), event->button());
}

void WaylandSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_surface)
        m_surface->sendMouseMoveEvent(toSurface(event->pos()));
}

void WaylandSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
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
    if (m_touchEventsEnabled && m_surface) {
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

void WaylandSurfaceItem::surfaceMapped(const QSize &)
{
    setPaintEnabled(true);
}

void WaylandSurfaceItem::surfaceUnmapped()
{
    setPaintEnabled(false);
}

void WaylandSurfaceItem::surfaceDestroyed(QObject *)
{
    m_surface = 0;
}

void WaylandSurfaceItem::surfaceDamaged(const QRect &)
{
    m_damaged = true;
    emit textureChanged();
    update();
}

void WaylandSurfaceItem::parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    WaylandSurfaceItem *item = newParent? newParent->surfaceItem():0;
    setParentItem(item);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void WaylandSurfaceItem::updateGeometry()
{
    setPos(m_surface->geometry().topLeft());
    setSize(m_surface->geometry().size());
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
    if (!m_surface) {
        delete oldNode;
        return 0;
    }

    if (m_damaged) {
        QSGTexture *oldTexture = m_texture;
        if (m_surface->type() == WaylandSurface::Texture) {
            QOpenGLContext *context = QOpenGLContext::currentContext();

            QQuickCanvas::CreateTextureOptions opt = useTextureAlpha() ? QQuickCanvas::TextureHasAlphaChannel : QQuickCanvas::CreateTextureOptions(0);
            m_texture = canvas()->createTextureFromId(m_surface->texture(context),
                                                                          m_surface->geometry().size(),
                                                                          opt);
        } else {
            m_texture = canvas()->createTextureFromImage(m_surface->image());
        }

        delete oldTexture;
        m_damaged = false;
    }

    if (m_provider) {
        m_provider->t = m_texture;
        m_provider->smooth = smooth();
    }

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

void WaylandSurfaceItem::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}
