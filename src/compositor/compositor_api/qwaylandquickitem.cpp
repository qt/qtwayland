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

#include "qwaylandquickitem.h"
#include "qwaylandquicksurface.h"
#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/qwaylandinput.h>
#include <QtWaylandCompositor/qwaylandbufferref.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>

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

QMutex *QWaylandQuickItem::mutex = 0;

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

    void setBufferRef(QWaylandQuickItem *surfaceItem, const QWaylandBufferRef &buffer)
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
                QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(surfaceItem->surface());
                if (surface && surface->useTextureAlpha()) {
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

QWaylandQuickItem::QWaylandQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_view(new QWaylandView(this, this))
    , m_oldSurface(Q_NULLPTR)
    , m_provider(Q_NULLPTR)
    , m_paintEnabled(true)
    , m_touchEventsEnabled(false)
    , m_resizeSurfaceToItem(false)
    , m_inputEventsEnabled(true)
    , m_newTexture(false)
    , m_focusOnClick(true)
    , m_connectedWindow(Q_NULLPTR)
    , m_origin(QWaylandSurface::OriginTopLeft)
{
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

    connect(this, &QQuickItem::windowChanged, this, &QWaylandQuickItem::updateWindow);
    connect(m_view.data(), &QWaylandView::surfaceChanged, this, &QWaylandQuickItem::surfaceChanged);
    connect(m_view.data(), &QWaylandView::surfaceChanged, this, &QWaylandQuickItem::handleSurfaceChanged);
}

QWaylandQuickItem::~QWaylandQuickItem()
{
    disconnect(this, &QQuickItem::windowChanged, this, &QWaylandQuickItem::updateWindow);
    QMutexLocker locker(mutex);
    if (m_provider)
        m_provider->deleteLater();
}

QWaylandCompositor *QWaylandQuickItem::compositor() const
{
    return m_view->surface() ? m_view->surface()->compositor() : Q_NULLPTR;
}

QWaylandView *QWaylandQuickItem::view() const
{
    return m_view.data();
}

QWaylandSurface *QWaylandQuickItem::surface() const
{
    return m_view->surface();
}

void QWaylandQuickItem::setSurface(QWaylandSurface *surface)
{
    m_view->setSurface(surface);
}

QWaylandSurface::Origin QWaylandQuickItem::origin() const
{
    return m_origin;
}

QSGTextureProvider *QWaylandQuickItem::textureProvider() const
{
    return m_provider;
}

void QWaylandQuickItem::mousePressEvent(QMouseEvent *event)
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

    if (m_focusOnClick)
        takeFocus(inputDevice);

    inputDevice->sendMousePressEvent(event->button());
}

void QWaylandQuickItem::mouseMoveEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(m_view.data(), event->localPos(), event->windowPos());
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button());
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::hoverEnterEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    }
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(m_view.data(), event->pos(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::hoverMoveEvent(QHoverEvent *event)
{
    if (surface()) {
        if (!surface()->inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    }
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(m_view.data(), event->pos(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::hoverLeaveEvent(QHoverEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->setMouseFocus(Q_NULLPTR);
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::wheelEvent(QWheelEvent *event)
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

void QWaylandQuickItem::keyPressEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::keyReleaseEvent(QKeyEvent *event)
{
    if (shouldSendInputEvents() && hasFocus()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::touchEvent(QTouchEvent *event)
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
        if (inputDevice->mouseFocus() != m_view.data()) {
            inputDevice->sendMouseMoveEvent(m_view.data(), pointPos, mapToScene(pointPos));
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

void QWaylandQuickItem::mouseUngrabEvent()
{
    if (surface()) {
        QTouchEvent e(QEvent::TouchCancel);
        touchEvent(&e);
    }
}

void QWaylandQuickItem::handleSurfaceChanged()
{
    if (m_oldSurface) {
        disconnect(m_oldSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        disconnect(m_oldSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        disconnect(m_oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        disconnect(m_oldSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        disconnect(m_oldSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
    }
    if (QWaylandSurface *newSurface = m_view->surface()) {
        connect(newSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        connect(newSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        connect(newSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        connect(newSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
        setWidth(newSurface->size().width());
        setHeight(newSurface->size().height());
        if (newSurface->origin() != m_origin) {
            m_origin = newSurface->origin();
            emit originChanged();
        }
        if (window()) {
            QWaylandOutput *output = newSurface->compositor()->output(window());
            m_view->setOutput(output);
        }
    }
    m_oldSurface = m_view->surface();
}

void QWaylandQuickItem::takeFocus(QWaylandInputDevice *device)
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

void QWaylandQuickItem::surfaceMappedChanged()
{
    update();
}

void QWaylandQuickItem::parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

void QWaylandQuickItem::updateSize()
{
    if (surface()) {
        setSize(surface()->size());
    }
}

void QWaylandQuickItem::syncGraphicsState()
{

}

bool QWaylandQuickItem::focusOnClick() const
{
    return m_focusOnClick;
}

void QWaylandQuickItem::setFocusOnClick(bool focus)
{
    if (m_focusOnClick == focus)
        return;

    m_focusOnClick = focus;
    emit focusOnClickChanged();
}

/*!
    \qmlproperty bool QtWayland::QWaylandSurfaceItem::paintEnabled

    If this property is true, the \l item is hidden, though the texture
    will still be updated. As opposed to hiding the \l item by
    setting \l{Item::visible}{visible} to false, setting this property to true
    will not prevent mouse or keyboard input from reaching \l item.
*/
bool QWaylandQuickItem::paintEnabled() const
{
    return m_paintEnabled;
}

void QWaylandQuickItem::setPaintEnabled(bool enabled)
{
    m_paintEnabled = enabled;
    update();
}

void QWaylandQuickItem::updateBuffer(bool hasBuffer)
{
    Q_UNUSED(hasBuffer);
    if (m_origin != surface()->origin()) {
        m_origin = surface()->origin();
        emit originChanged();
    }
}

void QWaylandQuickItem::updateWindow()
{
    if (m_connectedWindow) {
        disconnect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync);
    }

    m_connectedWindow = window();

    if (m_connectedWindow) {
        connect(m_connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync, Qt::DirectConnection);
    }

    if (compositor() && m_connectedWindow) {
        QWaylandOutput *output = compositor()->output(m_connectedWindow);
        Q_ASSERT(output);
        m_view->setOutput(output);
    }
}

void QWaylandQuickItem::beforeSync()
{
    if (m_view->advance()) {
        m_newTexture = true;
        update();
    }
}

QSGNode *QWaylandQuickItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    bool mapped = (surface() && surface()->isMapped() && m_view->currentBuffer().hasBuffer())
        || (m_view->isBufferLocked() && m_provider);

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
        m_provider->setBufferRef(this, m_view->currentBuffer());
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

void QWaylandQuickItem::setTouchEventsEnabled(bool enabled)
{
    if (m_touchEventsEnabled != enabled) {
        m_touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

void QWaylandQuickItem::setResizeSurfaceToItem(bool enabled)
{
    if (m_resizeSurfaceToItem != enabled) {
        m_resizeSurfaceToItem = enabled;
        emit resizeSurfaceToItemChanged();
    }
}

void QWaylandQuickItem::setInputEventsEnabled(bool enabled)
{
    if (m_inputEventsEnabled != enabled) {
        m_inputEventsEnabled = enabled;
        setAcceptHoverEvents(enabled);
        emit inputEventsEnabledChanged();
    }
}

QT_END_NAMESPACE
