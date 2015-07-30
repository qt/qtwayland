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

#include "qwaylandquickview.h"
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

QMutex *QWaylandQuickView::mutex = 0;

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

    void setBufferRef(QWaylandQuickView *surfaceItem, const QWaylandBufferRef &buffer)
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

QWaylandQuickView::QWaylandQuickView(QQuickItem *parent)
    : QQuickItem(parent)
    , QWaylandView()
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

    connect(this, &QQuickItem::windowChanged, this, &QWaylandQuickView::updateWindow);
}

QWaylandQuickView::~QWaylandQuickView()
{
    QMutexLocker locker(mutex);
    if (m_provider)
        m_provider->deleteLater();
}

QWaylandQuickSurface *QWaylandQuickView::surface() const
{
    return static_cast<QWaylandQuickSurface *>(QWaylandView::surface());
}

void QWaylandQuickView::setSurface(QWaylandQuickSurface *surface)
{
    QWaylandView::setSurface(surface);
}

QWaylandSurface::Origin QWaylandQuickView::origin() const
{
    return m_origin;
}

QSGTextureProvider *QWaylandQuickView::textureProvider() const
{
    return m_provider;
}

void QWaylandQuickView::mousePressEvent(QMouseEvent *event)
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
    inputDevice->sendMousePressEvent(event->button());
}

void QWaylandQuickView::mouseMoveEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->localPos(), event->windowPos());
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::mouseReleaseEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button());
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::hoverEnterEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(this, event->pos(), QPoint());
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::hoverMoveEvent(QHoverEvent *event)
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

void QWaylandQuickView::hoverLeaveEvent(QHoverEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->setMouseFocus(Q_NULLPTR);
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::wheelEvent(QWheelEvent *event)
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

void QWaylandQuickView::keyPressEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::keyReleaseEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents() && hasFocus()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandQuickView::touchEvent(QTouchEvent *event)
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
            inputDevice->sendMouseMoveEvent(this, pointPos, QPointF());
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

void QWaylandQuickView::waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    QWaylandView::waylandSurfaceChanged(newSurface, oldSurface);
    if (oldSurface) {
        disconnect(oldSurface, &QWaylandSurface::mapped, this, &QWaylandQuickView::surfaceMapped);
        disconnect(oldSurface, &QWaylandSurface::unmapped, this, &QWaylandQuickView::surfaceUnmapped);
        disconnect(oldSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickView::parentChanged);
        disconnect(oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickView::updateSize);
        disconnect(oldSurface, &QWaylandSurface::configure, this, &QWaylandQuickView::updateBuffer);
        disconnect(oldSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
    }
    if (newSurface) {
        connect(newSurface, &QWaylandSurface::mapped, this, &QWaylandQuickView::surfaceMapped);
        connect(newSurface, &QWaylandSurface::unmapped, this, &QWaylandQuickView::surfaceUnmapped);
        connect(newSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickView::parentChanged);
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickView::updateSize);
        connect(newSurface, &QWaylandSurface::configure, this, &QWaylandQuickView::updateBuffer);
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

void QWaylandQuickView::waylandSurfaceDestroyed()
{
    emit surfaceDestroyed();
}

void QWaylandQuickView::takeFocus(QWaylandInputDevice *device)
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

void QWaylandQuickView::surfaceMapped()
{
    update();
}

void QWaylandQuickView::surfaceUnmapped()
{
    update();
}

void QWaylandQuickView::parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void QWaylandQuickView::updateSize()
{
    if (surface()) {
        setSize(surface()->size());
    }
}

void QWaylandQuickView::setRequestedPosition(const QPointF &pos)
{
    bool xChanged = pos.x() != requestedPosition().x();
    bool yChanged = pos.y() != requestedPosition().y();
    QWaylandView::setRequestedPosition(pos);
    if (xChanged)
        emit requestedXPositionChanged();
    if (yChanged)
        emit requestedYPositionChanged();
    if (m_followRequestedPos)
        setPosition(pos);
}

QPointF QWaylandQuickView::pos() const
{
    return position();
}


bool QWaylandQuickView::followRequestedPosition() const
{
    return m_followRequestedPos;
}

void QWaylandQuickView::setFollowRequestedPosition(bool follow)
{
    if (m_followRequestedPos != follow) {
        m_followRequestedPos = follow;
        emit followRequestedPositionChanged();
    }
}

qreal QWaylandQuickView::requestedXPosition() const
{
    return requestedPosition().x();
}

void QWaylandQuickView::setRequestedXPosition(qreal xPos)
{
    QPointF reqPos = requestedPosition();
    reqPos.setX(xPos);
    setRequestedPosition(reqPos);
}

qreal QWaylandQuickView::requestedYPosition() const
{
    return requestedPosition().y();
}

void QWaylandQuickView::setRequestedYPosition(qreal yPos)
{
    QPointF reqPos = requestedPosition();
    reqPos.setY(yPos);
    setRequestedPosition(reqPos);
}

void QWaylandQuickView::syncGraphicsState()
{

}

bool QWaylandQuickView::lockedBuffer() const
{
    return QWaylandView::lockedBuffer();
}

void QWaylandQuickView::setLockedBuffer(bool locked)
{
    if (locked != lockedBuffer()) {
        QWaylandView::setLockedBuffer(locked);
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
bool QWaylandQuickView::paintEnabled() const
{
    return m_paintEnabled;
}

void QWaylandQuickView::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

void QWaylandQuickView::updateBuffer(bool hasBuffer)
{
    Q_UNUSED(hasBuffer);
    if (m_origin != surface()->origin()) {
        m_origin = surface()->origin();
        emit originChanged();
    }
}

void QWaylandQuickView::updateWindow()
{
    if (m_connectedWindow) {
        disconnect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickView::beforeSync);
    }

    m_connectedWindow = window();

    if (m_connectedWindow) {
        connect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickView::beforeSync, Qt::DirectConnection);
    }

    if (compositor() && m_connectedWindow) {
        QWaylandOutput *output = compositor()->output(m_connectedWindow);
        Q_ASSERT(output);
        setOutput(output);
    }
}

void QWaylandQuickView::beforeSync()
{
    if (advance()) {
        m_newTexture = true;
        update();
    }
}

QSGNode *QWaylandQuickView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
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

void QWaylandQuickView::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

void QWaylandQuickView::setResizeSurfaceToItem(bool enabled)
{
    if (m_resizeSurfaceToItem != enabled) {
        m_resizeSurfaceToItem = enabled;
        emit resizeSurfaceToItemChanged();
    }
}

void QWaylandQuickView::setInputEventsEnabled(bool enabled)
{
    if (m_inputEventsEnabled != enabled) {
        m_inputEventsEnabled = enabled;
        setAcceptHoverEvents(enabled);
        emit inputEventsEnabledChanged();
    }
}

QT_END_NAMESPACE
