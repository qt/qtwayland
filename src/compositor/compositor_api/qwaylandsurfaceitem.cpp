/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

#include "qwaylandsurfaceitem.h"
#include "qwaylandsurfacenode_p.h"
#include "qwaylandsurface.h"
#include "qwaylandcompositor.h"
#include "qwaylandinput.h"

#include "qwlsurface_p.h"
#include "qwlextendedsurface_p.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleRectNode>
#include <QtQuick/QQuickWindow>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

class QWaylandSurfaceTextureProvider : public QSGTextureProvider
{
    Q_OBJECT

public:
    QWaylandSurfaceTextureProvider() : t(0) { }
    ~QWaylandSurfaceTextureProvider() { delete t; }

    QSGTexture *texture() const {
        if (t)
            t->setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return t;
    }

    bool smooth;
    QSGTexture *t;

public slots:
    void invalidate()
    {
        delete t;
        t = 0;
    }
};

QMutex *QWaylandSurfaceItem::mutex = 0;

QWaylandSurfaceItem::QWaylandSurfaceItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_provider(0)
    , m_node(0)
    , m_paintEnabled(true)
    , m_useTextureAlpha(false)
    , m_clientRenderingEnabled(false)
    , m_touchEventsEnabled(false)
    , m_resizeSurfaceToItem(false)
{
    if (!mutex)
        mutex = new QMutex;
}

QWaylandSurfaceItem::QWaylandSurfaceItem(QWaylandSurface *surface, QQuickItem *parent)
    : QQuickItem(parent)
    , m_surface(0)
    , m_provider(0)
    , m_node(0)
    , m_paintEnabled(true)
    , m_useTextureAlpha(false)
    , m_clientRenderingEnabled(false)
    , m_touchEventsEnabled(false)
{
    init(surface);
}

void QWaylandSurfaceItem::init(QWaylandSurface *surface)
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

    if (m_resizeSurfaceToItem) {
        updateSurfaceSize();
    } else {
        setWidth(surface->size().width());
        setHeight(surface->size().height());
    }

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
        Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
        Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
        Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
        Qt::ExtraButton12 | Qt::ExtraButton13);
    setAcceptHoverEvents(true);
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    connect(surface, SIGNAL(parentChanged(QWaylandSurface*,QWaylandSurface*)),
            this, SLOT(parentChanged(QWaylandSurface*,QWaylandSurface*)));
    connect(surface, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
    connect(surface, SIGNAL(posChanged()), this, SLOT(updatePosition()));
    connect(this, SIGNAL(widthChanged()), this, SLOT(updateSurfaceSize()));
    connect(this, SIGNAL(heightChanged()), this, SLOT(updateSurfaceSize()));

    m_damaged = false;
    m_yInverted = surface ? surface->isYInverted() : true;
}

QWaylandSurfaceItem::~QWaylandSurfaceItem()
{
    QMutexLocker locker(mutex);
    if (m_node)
        m_node->setItem(0);
    if (m_surface)
        m_surface->setSurfaceItem(0);
    if (m_provider)
        m_provider->deleteLater();
}

void QWaylandSurfaceItem::setSurface(QWaylandSurface *surface)
{
    if (surface == m_surface)
        return;

    init(surface);
    emit surfaceChanged();
}

bool QWaylandSurfaceItem::isYInverted() const
{
    return m_yInverted;
}

QSGTextureProvider *QWaylandSurfaceItem::textureProvider() const
{
    const_cast<QWaylandSurfaceItem *>(this)->ensureProvider();
    return m_provider;
}

void QWaylandSurfaceItem::ensureProvider()
{
    if (!m_provider) {
        m_provider = new QWaylandSurfaceTextureProvider();
        connect(window(), SIGNAL(sceneGraphInvalidated()), m_provider, SLOT(invalidate()), Qt::DirectConnection);
    }
}

void QWaylandSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (m_surface) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        if (inputDevice->mouseFocus() != m_surface)
            inputDevice->setMouseFocus(m_surface, event->pos(), event->globalPos());
        inputDevice->sendMousePressEvent(event->button(), toSurface(event->pos()), event->globalPos());
    }
}

void QWaylandSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_surface){
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendMouseMoveEvent(m_surface, toSurface(event->pos()), event->globalPos());
    }
}

void QWaylandSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_surface){
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendMouseReleaseEvent(event->button(), toSurface(event->pos()), event->globalPos());
    }
}

void QWaylandSurfaceItem::wheelEvent(QWheelEvent *event)
{
    if (m_surface) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendMouseWheelEvent(event->orientation(), event->delta());
    }
}

void QWaylandSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus()) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendFullKeyEvent(event);
    }
}

void QWaylandSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (m_surface && hasFocus()) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->sendFullKeyEvent(event);
    }
}

void QWaylandSurfaceItem::touchEvent(QTouchEvent *event)
{
    if (m_touchEventsEnabled && m_surface) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
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

void QWaylandSurfaceItem::takeFocus()
{
    setFocus(true);

    if (m_surface) {
        QWaylandInputDevice *inputDevice = m_surface->compositor()->defaultInputDevice();
        inputDevice->setKeyboardFocus(m_surface);
    }
}

QPoint QWaylandSurfaceItem::toSurface(const QPointF &pos) const
{
    return pos.toPoint();
}

void QWaylandSurfaceItem::surfaceMapped()
{
    setPaintEnabled(true);
}

void QWaylandSurfaceItem::surfaceUnmapped()
{
    setPaintEnabled(false);
}

void QWaylandSurfaceItem::surfaceDestroyed(QObject *)
{
    m_surface = 0;
}

void QWaylandSurfaceItem::setDamagedFlag(bool on)
{
    m_damaged = on;
}


void QWaylandSurfaceItem::surfaceDamaged(const QRect &)
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

void QWaylandSurfaceItem::parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    QWaylandSurfaceItem *item = newParent? newParent->surfaceItem():0;
    setParentItem(item);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void QWaylandSurfaceItem::updateSize()
{
    setSize(m_surface->size());
}

void QWaylandSurfaceItem::updateSurfaceSize()
{
    if (m_resizeSurfaceToItem) {
        m_surface->sendConfigure(QSize(width(), height()));
    }
}

void QWaylandSurfaceItem::updatePosition()
{
    setPosition(m_surface->pos());
}

bool QWaylandSurfaceItem::paintEnabled() const
{
    return m_paintEnabled;
}

void QWaylandSurfaceItem::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

void QWaylandSurfaceItem::updateTexture()
{
    ensureProvider();
    QSGTexture *texture = m_provider->t;
    if (m_damaged) {
        m_damaged = false;
        QSGTexture *oldTexture = texture;
        if (m_surface->type() == QWaylandSurface::Texture) {
            QOpenGLContext *context = QOpenGLContext::currentContext();
            QQuickWindow::CreateTextureOptions opt = 0;
            if (useTextureAlpha()) {
                opt |= QQuickWindow::TextureHasAlphaChannel;
            }
            texture = window()->createTextureFromId(m_surface->texture(context), m_surface->size(), opt);
        } else {
            texture = window()->createTextureFromImage(m_surface->image());
        }
        texture->bind();
        delete oldTexture;
    }

    m_provider->t = texture;
    emit m_provider->textureChanged();
    m_provider->smooth = smooth();
}

QSGNode *QWaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_surface) {
        delete oldNode;
        return 0;
    }

    updateTexture();
    if (!m_provider->t || !m_paintEnabled) {
        delete oldNode;
        return 0;
    }

    QWaylandSurfaceNode *node = static_cast<QWaylandSurfaceNode *>(oldNode);

    if (!node) {
        node = new QWaylandSurfaceNode(this);
    }

    node->updateTexture();

    if (surface()->isYInverted()) {
        node->setRect(0, height(), width(), -height());
    } else {
        node->setRect(0, 0, width(), height());
    }

    node->setTextureUpdated(true);

    return node;
}

void QWaylandSurfaceItem::setUseTextureAlpha(bool useTextureAlpha)
{
    m_useTextureAlpha = useTextureAlpha;

    if ((flags() & ItemHasContents) != 0) {
        update();
    }
}

void QWaylandSurfaceItem::setClientRenderingEnabled(bool enabled)
{
    if (m_clientRenderingEnabled != enabled) {
        m_clientRenderingEnabled = enabled;

        if (m_surface) {
            m_surface->sendOnScreenVisibilityChange(enabled);
        }

        emit clientRenderingEnabledChanged();
    }
}

void QWaylandSurfaceItem::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

void QWaylandSurfaceItem::setResizeSurfaceToItem(bool enabled)
{
    if (m_resizeSurfaceToItem != enabled) {
        m_resizeSurfaceToItem = enabled;
        emit resizeSurfaceToItemChanged();
    }
}

#include "qwaylandsurfaceitem.moc"

QT_END_NAMESPACE
