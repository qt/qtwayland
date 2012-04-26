/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Compositor.
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#include "waylandsurfaceitem.h"
#include "waylandsurface.h"
#include "waylandcompositor.h"
#include "waylandinput.h"

#include "wlsurface.h"
#include "wlextendedsurface.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QSGSimpleRectNode>
#include <QtQuick/QQuickCanvas>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

class WaylandSurfaceNode : public QSGSimpleTextureNode
{
public:
    WaylandSurfaceNode(WaylandSurfaceItem *item) : m_item(item), smooth(true), t(0) {
        if (m_item)
            m_item->m_node = this;
        setFlag(UsePreprocess,true);
    }
    ~WaylandSurfaceNode() {
        QMutexLocker locker(WaylandSurfaceItem::mutex);
        delete t;
        if (m_item)
            m_item->m_node = 0;
    }
    void preprocess() {
        QMutexLocker locker(WaylandSurfaceItem::mutex);
        if (m_item && m_item->m_damaged) {
            QSGTexture *newTexture = m_item->updateTexture(t);
            updateTexture(newTexture);
        }
    }

    void updateTexture(QSGTexture *texture) {
        t = texture;
        setTexture(texture);
        setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
    }

    WaylandSurfaceItem *m_item;
    bool smooth;
    QSGTexture *t;
};

class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    WaylandSurfaceTextureProvider() : node(0) { }

    QSGTexture *texture() const {
        if (node && node->t) {
            node->t->setFiltering(node->smooth ? QSGTexture::Linear : QSGTexture::Nearest);
            return node->t;
        }
        return 0;
    }

    WaylandSurfaceNode *node;
};

QMutex *WaylandSurfaceItem::mutex = 0;

WaylandSurfaceItem::WaylandSurfaceItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_provider(0)
    , m_paintEnabled(true)
    , m_useTextureAlpha(false)
    , m_clientRenderingEnabled(false)
    , m_touchEventsEnabled(false)
{
    if (!mutex)
        mutex = new QMutex;
}

WaylandSurfaceItem::WaylandSurfaceItem(WaylandSurface *surface, QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_provider(0)
    , m_paintEnabled(true)
    , m_useTextureAlpha(false)
    , m_clientRenderingEnabled(false)
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
    if (m_clientRenderingEnabled) {
        m_surface->sendOnScreenVisibilityChange(m_clientRenderingEnabled);
    }

    setWidth(surface->size().width());
    setHeight(surface->size().height());

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    connect(surface, SIGNAL(parentChanged(WaylandSurface*,WaylandSurface*)),
            this, SLOT(parentChanged(WaylandSurface*,WaylandSurface*)));
    connect(surface, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
    connect(surface, SIGNAL(posChanged()), this, SLOT(updatePosition()));

    m_damaged = false;
    m_yInverted = surface ? surface->isYInverted() : true;
}

WaylandSurfaceItem::~WaylandSurfaceItem()
{
    QMutexLocker locker(mutex);
    if (m_node)
        m_node->m_item = 0;
    if (m_surface) {
        m_surface->setSurfaceItem(0);
    }
}

void WaylandSurfaceItem::setSurface(WaylandSurface *surface)
{
    if (surface == m_surface)
        return;

    init(surface);
    emit surfaceChanged();
}

bool WaylandSurfaceItem::isYInverted() const
{
    return m_yInverted;
}

QSGTextureProvider *WaylandSurfaceItem::textureProvider() const
{
    if (!m_provider)
        m_provider = new WaylandSurfaceTextureProvider();
    return m_provider;
}

void WaylandSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (m_surface) {
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        if (inputDevice->mouseFocus() != m_surface)
            inputDevice->setMouseFocus(m_surface, event->pos(), event->globalPos());
        inputDevice->sendMousePressEvent(event->button(), toSurface(event->pos()));
    }
}

void WaylandSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_surface){
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendMouseMoveEvent(m_surface, toSurface(event->pos()));
    }
}

void WaylandSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_surface){
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendMouseReleaseEvent(event->button(), toSurface(event->pos()));
    }
}

void WaylandSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus()) {
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendFullKeyEvent(event);
    }
}

void WaylandSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus()) {
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendFullKeyEvent(event);
    }
}

void WaylandSurfaceItem::touchEvent(QTouchEvent *event)
{
    if (m_touchEventsEnabled && m_surface) {
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        event->accept();
        if (inputDevice->mouseFocus() != m_surface) {
            QPoint pointPos;
            QList<QTouchEvent::TouchPoint> points = event->touchPoints();
            if (!points.isEmpty())
                pointPos = points.at(0).pos().toPoint();
            inputDevice->setMouseFocus(m_surface, pointPos, pointPos);
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

void WaylandSurfaceItem::takeFocus()
{
    setFocus(true);

    if (m_surface) {
        WaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->setKeyboardFocus(m_surface);
    }
}

QPoint WaylandSurfaceItem::toSurface(const QPointF &pos) const
{
    return pos.toPoint();
}

void WaylandSurfaceItem::surfaceMapped()
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

void WaylandSurfaceItem::setDamagedFlag(bool on)
{
    m_damaged = on;
}


void WaylandSurfaceItem::surfaceDamaged(const QRect &)
{
    m_damaged = true;
    if (m_surface) {
        bool inverted = m_surface->isYInverted();
        if (inverted  != m_yInverted) {
            m_yInverted = inverted;
            emit yInvertedChanged();
        }
    }
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

void WaylandSurfaceItem::updateSize()
{
    setSize(m_surface->size());
}

void WaylandSurfaceItem::updatePosition()
{
    setPos(m_surface->pos());
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


QSGTexture *WaylandSurfaceItem::updateTexture(QSGTexture *oldTexture)
{
    QSGTexture *newTexture;
    if (m_damaged) {
        m_damaged = false;
        if (m_surface->type() == WaylandSurface::Texture) {
            QOpenGLContext *context = QOpenGLContext::currentContext();

            QQuickCanvas::CreateTextureOptions opt = useTextureAlpha() ? QQuickCanvas::TextureHasAlphaChannel : QQuickCanvas::CreateTextureOptions(0);
            newTexture = canvas()->createTextureFromId(m_surface->texture(context),
                                                                          m_surface->size(),
                                                                          opt);
        } else {
            newTexture = canvas()->createTextureFromImage(m_surface->image());
        }

        delete oldTexture;
    } else {
        newTexture = oldTexture;
    }
    return newTexture;
}


QSGNode *WaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_surface) {
        delete oldNode;
        return 0;
    }

    WaylandSurfaceNode *node = static_cast<WaylandSurfaceNode *>(oldNode);

    QSGTexture *oldTexture = node ? node->t : 0;
    QSGTexture *newTexture = updateTexture(oldTexture);

    if (!newTexture || !m_paintEnabled) {
        delete oldNode;
        if (m_provider)
            m_provider->node = 0;
        return 0;
    }

    if (!node) {
        node = new WaylandSurfaceNode(this);
    }

    node->updateTexture(newTexture);

    if (surface()->isYInverted()) {
        node->setRect(0, height(), width(), -height());
    } else {
        node->setRect(0, 0, width(), height());
    }

    node->smooth = smooth();
    if (m_provider)
        m_provider->node = node;

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
