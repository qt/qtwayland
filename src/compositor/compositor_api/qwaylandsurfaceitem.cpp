/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandsurfaceitem.h"
#include "qwaylandquicksurface.h"
#include <QtCompositor/qwaylandcompositor.h>
#include <QtCompositor/qwaylandinput.h>
#include <QtCompositor/qwaylandbufferref.h>
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlclientbufferintegration_p.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QQuickWindow>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

#include <wayland-server.h>
#include <QThread>
QT_BEGIN_NAMESPACE

QMutex *QWaylandSurfaceItem::mutex = 0;

class QWaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    QWaylandSurfaceTextureProvider()
        : m_smooth(false)
        , m_sgTex(0)
    {
    }

    ~QWaylandSurfaceTextureProvider()
    {
        if (m_sgTex)
            m_sgTex->deleteLater();
    }

    void setBufferRef(QWaylandSurfaceItem *surfaceItem, const QWaylandBufferRef &buffer)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        m_ref = buffer;
        delete m_sgTex;
        m_sgTex = 0;
        if (m_ref.hasBuffer()) {
            if (buffer.isShm()) {
                m_sgTex = surfaceItem->window()->createTextureFromImage(buffer.image());
                m_invertY = false;
                if (m_sgTex) {
                    m_sgTex->bind();
                }
            } else {
                QQuickWindow::CreateTextureOptions opt = QQuickWindow::TextureOwnsGLTexture;
                if (surfaceItem->surface()->useTextureAlpha()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }

                GLuint texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                buffer.bindToTexture();
                m_sgTex = surfaceItem->window()->createTextureFromId(texture , QSize(surfaceItem->width(), surfaceItem->height()), opt);
                m_invertY = buffer.origin() == QWaylandSurface::OriginBottomLeft;
            }
        }
        emit textureChanged();
    }

    QSGTexture *texture() const Q_DECL_OVERRIDE
    {
        if (m_sgTex)
            m_sgTex->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return m_sgTex;
    }

    void setSmooth(bool smooth) { m_smooth = smooth; }
    bool invertY() const { return m_invertY; }
private:
    bool m_smooth;
    bool m_invertY;
    QSGTexture *m_sgTex;
    QWaylandBufferRef m_ref;
};

QWaylandSurfaceItem::QWaylandSurfaceItem(QQuickItem *parent)
    : QQuickItem(parent)
    , QWaylandSurfaceView()
    , m_provider(Q_NULLPTR)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
    , m_resizeSurfaceToItem(false)
    , m_followRequestedPos(true)
    , m_inputEventsEnabled(true)
    , m_newTexture(false)
    , m_connectedWindow(Q_NULLPTR)
    , m_origin(QWaylandSurface::OriginTopLeft)
{
    setAcceptHoverEvents(true);
    if (!mutex)
        mutex = new QMutex;

    setFlag(ItemHasContents);

    update();

    setSmooth(true);

    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
        Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
        Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
        Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
        Qt::ExtraButton12 | Qt::ExtraButton13);
    setAcceptHoverEvents(true);

    connect(this, &QQuickItem::windowChanged, this, &QWaylandSurfaceItem::updateWindow);
}

QWaylandSurfaceItem::~QWaylandSurfaceItem()
{
    QMutexLocker locker(mutex);
    if (m_provider)
        m_provider->deleteLater();
}

QWaylandQuickSurface *QWaylandSurfaceItem::surface() const
{
    return static_cast<QWaylandQuickSurface *>(QWaylandSurfaceView::surface());
}

void QWaylandSurfaceItem::setSurface(QWaylandQuickSurface *surface)
{
    QWaylandSurfaceView::setSurface(surface);
}

QWaylandSurface::Origin QWaylandSurfaceItem::origin() const
{
    return m_origin;
}

QSGTextureProvider *QWaylandSurfaceItem::textureProvider() const
{
    return m_provider;
}

void QWaylandSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (!shouldSendInputEvents()) {
        event->ignore();
        return;
    }

    if (!surface()->inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }

    QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
    if (inputDevice->mouseFocus() != this)
        inputDevice->setMouseFocus(this, event->localPos(), event->windowPos());
    inputDevice->sendMousePressEvent(event->button(), event->localPos(), event->windowPos());
}

void QWaylandSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->localPos(), event->windowPos());
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button(), event->localPos(), event->windowPos());
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::hoverEnterEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseEnterEvent(this, event->pos());
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::hoverMoveEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->pos());
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::hoverLeaveEvent(QHoverEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseLeaveEvent(this);
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::wheelEvent(QWheelEvent *event)
{
    if (shouldSendInputEvents()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }

        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseWheelEvent(event->orientation(), event->delta());
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::keyPressEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::keyReleaseEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents() && hasFocus()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::touchEvent(QTouchEvent *event)
{
    if (shouldSendInputEvents() && m_touchEventsEnabled) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);

        if (event->type() == QEvent::TouchBegin) {
            QQuickItem *grabber = window()->mouseGrabberItem();
            if (grabber != this)
                grabMouse();
        }

        QPoint pointPos;
        const QList<QTouchEvent::TouchPoint> &points = event->touchPoints();
        if (!points.isEmpty())
            pointPos = points.at(0).pos().toPoint();

        if (event->type() == QEvent::TouchBegin && !surface()->inputRegionContains(pointPos)) {
            event->ignore();
            return;
        }

        event->accept();
        if (inputDevice->mouseFocus() != this) {
            inputDevice->setMouseFocus(this, pointPos, pointPos);
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandSurfaceItem::mouseUngrabEvent()
{
    if (surface()) {
        QTouchEvent e(QEvent::TouchCancel);
        touchEvent(&e);
    }
}

void QWaylandSurfaceItem::waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    QWaylandSurfaceView::waylandSurfaceChanged(newSurface, oldSurface);
    if (oldSurface) {
        disconnect(oldSurface, &QWaylandSurface::mapped, this, &QWaylandSurfaceItem::surfaceMapped);
        disconnect(oldSurface, &QWaylandSurface::unmapped, this, &QWaylandSurfaceItem::surfaceUnmapped);
        disconnect(oldSurface, &QWaylandSurface::parentChanged, this, &QWaylandSurfaceItem::parentChanged);
        disconnect(oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandSurfaceItem::updateSize);
        disconnect(oldSurface, &QWaylandSurface::configure, this, &QWaylandSurfaceItem::updateBuffer);
        disconnect(oldSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
    }
    if (newSurface) {
        connect(newSurface, &QWaylandSurface::mapped, this, &QWaylandSurfaceItem::surfaceMapped);
        connect(newSurface, &QWaylandSurface::unmapped, this, &QWaylandSurfaceItem::surfaceUnmapped);
        connect(newSurface, &QWaylandSurface::parentChanged, this, &QWaylandSurfaceItem::parentChanged);
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandSurfaceItem::updateSize);
        connect(newSurface, &QWaylandSurface::configure, this, &QWaylandSurfaceItem::updateBuffer);
        connect(newSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
        setWidth(surface()->size().width());
        setHeight(surface()->size().height());
        if (newSurface->origin() != m_origin) {
            m_origin = newSurface->origin();
            emit originChanged();
        }
    }

    emit surfaceChanged();
}

void QWaylandSurfaceItem::waylandSurfaceDestroyed()
{
    emit surfaceDestroyed();
}

void QWaylandSurfaceItem::takeFocus(QWaylandInputDevice *device)
{
    setFocus(true);

    if (!surface())
        return;

    QWaylandInputDevice *target = device;
    if (!target) {
        target = compositor()->defaultInputDevice();
    }
    target->setKeyboardFocus(surface());
}

void QWaylandSurfaceItem::surfaceMapped()
{
    update();
}

void QWaylandSurfaceItem::surfaceUnmapped()
{
    update();
}

void QWaylandSurfaceItem::parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void QWaylandSurfaceItem::updateSize()
{
    if (surface()) {
        setSize(surface()->size());
    }
}

void QWaylandSurfaceItem::geometryChanged(const QRectF &newGeometry,
                                          const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (surface() && m_resizeSurfaceToItem) {
        surface()->requestSize(newGeometry.size().toSize());
    }
}

void QWaylandSurfaceItem::setRequestedPosition(const QPointF &pos)
{
    bool xChanged = pos.x() != requestedPosition().x();
    bool yChanged = pos.y() != requestedPosition().y();
    QWaylandSurfaceView::setRequestedPosition(pos);
    if (xChanged)
        emit requestedXPositionChanged();
    if (yChanged)
        emit requestedYPositionChanged();
    if (m_followRequestedPos)
        setPosition(pos);
}

QPointF QWaylandSurfaceItem::pos() const
{
    return position();
}


bool QWaylandSurfaceItem::followRequestedPosition() const
{
    return m_followRequestedPos;
}

void QWaylandSurfaceItem::setFollowRequestedPosition(bool follow)
{
    if (m_followRequestedPos != follow) {
        m_followRequestedPos = follow;
        emit followRequestedPositionChanged();
    }
}

qreal QWaylandSurfaceItem::requestedXPosition() const
{
    return requestedPosition().x();
}

void QWaylandSurfaceItem::setRequestedXPosition(qreal xPos)
{
    QPointF reqPos = requestedPosition();
    reqPos.setX(xPos);
    setRequestedPosition(reqPos);
}

qreal QWaylandSurfaceItem::requestedYPosition() const
{
    return requestedPosition().y();
}

void QWaylandSurfaceItem::setRequestedYPosition(qreal yPos)
{
    QPointF reqPos = requestedPosition();
    reqPos.setY(yPos);
    setRequestedPosition(reqPos);
}

void QWaylandSurfaceItem::syncGraphicsState()
{

}

bool QWaylandSurfaceItem::lockedBuffer() const
{
    return QWaylandSurfaceView::lockedBuffer();
}

void QWaylandSurfaceItem::setLockedBuffer(bool locked)
{
    if (locked != lockedBuffer()) {
        QWaylandSurfaceView::setLockedBuffer(locked);
        lockedBufferChanged();
    }
}

/*!
    \qmlproperty bool QtWayland::QWaylandSurfaceItem::paintEnabled

    If this property is true, the \l item is hidden, though the texture
    will still be updated. As opposed to hiding the \l item by
    setting \l{Item::visible}{visible} to false, setting this property to true
    will not prevent mouse or keyboard input from reaching \l item.
*/
bool QWaylandSurfaceItem::paintEnabled() const
{
    return m_paintEnabled;
}

void QWaylandSurfaceItem::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

void QWaylandSurfaceItem::updateBuffer(bool hasBuffer)
{
    Q_UNUSED(hasBuffer);
    if (m_origin != surface()->origin()) {
        m_origin = surface()->origin();
        emit originChanged();
    }
}

void QWaylandSurfaceItem::updateWindow()
{
    if (m_connectedWindow) {
        disconnect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandSurfaceItem::beforeSync);
    }

    m_connectedWindow = window();

    if (m_connectedWindow) {
        connect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandSurfaceItem::beforeSync, Qt::DirectConnection);
    }

    if (compositor() && m_connectedWindow) {
        QWaylandOutput *output = compositor()->output(m_connectedWindow);
        Q_ASSERT(output);
        setOutput(output);
    }
}

void QWaylandSurfaceItem::beforeSync()
{
    if (advance()) {
        m_newTexture = true;
        update();
    }
}

QSGNode *QWaylandSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    bool mapped = (surface() && surface()->isMapped() && currentBuffer().hasBuffer())
        || (lockedBuffer() && m_provider);

    if (!mapped || !m_paintEnabled) {
        delete oldNode;
        return 0;
    }

    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node)
        node = new QSGSimpleTextureNode();

    if (!m_provider)
        m_provider = new QWaylandSurfaceTextureProvider();

    if (m_newTexture) {
        m_newTexture = false;
        m_provider->setBufferRef(this, currentBuffer());
        node->setTexture(m_provider->texture());
    }

    m_provider->setSmooth(smooth());

    if (m_provider->invertY()) {
            node->setRect(0, height(), width(), -height());
    } else {
            node->setRect(0, 0, width(), height());
    }

    return node;
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

void QWaylandSurfaceItem::setInputEventsEnabled(bool enabled)
{
    if (m_inputEventsEnabled != enabled) {
        m_inputEventsEnabled = enabled;
        setAcceptHoverEvents(enabled);
        emit inputEventsEnabledChanged();
    }
}

QT_END_NAMESPACE
