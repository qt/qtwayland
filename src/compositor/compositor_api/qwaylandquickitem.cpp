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
#include "qwaylandquickitem_p.h"
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

QMutex *QWaylandQuickItemPrivate::mutex = 0;

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

/*!
 * \qmltype WaylandQuickItem
 * \inqmlmodule QtWayland.Compositor
 * \brief A Qt Quick item representing a WaylandView.
 *
 * When writing a WaylandCompositor in Qt Quick, this type can be used to display a
 * client's contents on an output device and will pass user input to the
 * client.
 */

/*!
 * \class QWaylandQuickItem
 * \inmodule QtWaylandCompositor
 * \brief A Qt Quick item representing a QWaylandView.
 *
 * When writing a QWaylandCompositor in Qt Quick, this class can be used to display a
 * client's contents on an output device and will pass user input to the
 * client.
 */

/*!
 * Constructs a QWaylandQuickItem with the given \a parent.
 */
QWaylandQuickItem::QWaylandQuickItem(QQuickItem *parent)
    : QQuickItem(*new QWaylandQuickItemPrivate(), parent)
{
    d_func()->init();
}

/*!
 * \internal
 */
QWaylandQuickItem::QWaylandQuickItem(QWaylandQuickItemPrivate &dd, QQuickItem *parent)
    : QQuickItem(dd, parent)
{
    d_func()->init();
}

/*!
 * Destroy the QWaylandQuickItem.
 */
QWaylandQuickItem::~QWaylandQuickItem()
{
    Q_D(QWaylandQuickItem);
    disconnect(this, &QQuickItem::windowChanged, this, &QWaylandQuickItem::updateWindow);
    QMutexLocker locker(d->mutex);
    if (d->provider)
        d->provider->deleteLater();
}

/*!
 * \qmlproperty object QtWaylandCompositor::WaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this QWaylandQuickItem.
 */
QWaylandCompositor *QWaylandQuickItem::compositor() const
{
    Q_D(const QWaylandQuickItem);
    return d->view->surface() ? d->view->surface()->compositor() : Q_NULLPTR;
}

/*!
 * \qmlproperty object QWaylandQuickItem::view
 *
 * This property holds the view rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::view
 *
 * This property holds the view rendered by this QWaylandQuickItem.
 */
QWaylandView *QWaylandQuickItem::view() const
{
    Q_D(const QWaylandQuickItem);
    return d->view.data();
}

/*!
 * \qmlproperty object QWaylandQuickItem::surface
 *
 * This property holds the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::surface
 *
 * This property holds the surface rendered by this QWaylandQuickItem.
 */

QWaylandSurface *QWaylandQuickItem::surface() const
{
    Q_D(const QWaylandQuickItem);
    return d->view->surface();
}

void QWaylandQuickItem::setSurface(QWaylandSurface *surface)
{
    Q_D(QWaylandQuickItem);
    d->view->setSurface(surface);
    update();
}

/*!
 * \qmlproperty enum QtWaylandCompositor::WaylandQuickItem::origin
 *
 * This property holds the origin of the QWaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::origin
 *
 * This property holds the origin of the QWaylandQuickItem.
 */
QWaylandSurface::Origin QWaylandQuickItem::origin() const
{
    Q_D(const QWaylandQuickItem);
    return d->origin;
}

/*!
 * Returns the texture provider of this QWaylandQuickItem.
 */
QSGTextureProvider *QWaylandQuickItem::textureProvider() const
{
    Q_D(const QWaylandQuickItem);

    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

    return d->provider;
}

/*!
 * \internal
 */
void QWaylandQuickItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (!d->shouldSendInputEvents()) {
        event->ignore();
        return;
    }

    if (!inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }

    QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);

    if (d->focusOnClick)
        takeFocus(inputDevice);

    inputDevice->sendMouseMoveEvent(d->view.data(), event->localPos(), event->windowPos());
    inputDevice->sendMousePressEvent(event->button());
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->localPos(), event->windowPos());
    } else {
        emit mouseMove(event->windowPos());
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button());
    } else {
        emit mouseRelease();
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (!inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->pos(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (surface()) {
        if (!inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    }
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->pos(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->setMouseFocus(Q_NULLPTR);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::wheelEvent(QWheelEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        if (!inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }

        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseWheelEvent(event->orientation(), event->delta());
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::keyPressEvent(QKeyEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents() && hasFocus()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::touchEvent(QTouchEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents() && d->touchEventsEnabled) {
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

        if (event->type() == QEvent::TouchBegin && !inputRegionContains(pointPos)) {
            event->ignore();
            return;
        }

        event->accept();
        if (inputDevice->mouseFocus() != d->view.data()) {
            inputDevice->sendMouseMoveEvent(d->view.data(), pointPos, mapToScene(pointPos));
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseUngrabEvent()
{
    if (surface()) {
        QTouchEvent e(QEvent::TouchCancel);
        touchEvent(&e);
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_UNUSED(newSurface);
    Q_UNUSED(oldSurface);
}

/*!
 * \internal
 */
void QWaylandQuickItem::handleSurfaceChanged()
{
    Q_D(QWaylandQuickItem);
    if (d->oldSurface) {
        disconnect(d->oldSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        disconnect(d->oldSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        disconnect(d->oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        disconnect(d->oldSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        disconnect(d->oldSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
    }
    if (QWaylandSurface *newSurface = d->view->surface()) {
        connect(newSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        connect(newSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        connect(newSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        connect(newSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
        if (d->sizeFollowsSurface) {
            setWidth(newSurface->size().width());
            setHeight(newSurface->size().height());
        }
        if (newSurface->origin() != d->origin) {
            d->origin = newSurface->origin();
            emit originChanged();
        }
        if (window()) {
            QWaylandOutput *output = newSurface->compositor()->outputFor(window());
            d->view->setOutput(output);
        }
    }
    surfaceChangedEvent(d->view->surface(), d->oldSurface);
    d->oldSurface = d->view->surface();
}

/*!
 * Calling this function causes the item to take the focus of the
 * input \a device.
 */
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

/*!
 * \internal
 */
void QWaylandQuickItem::surfaceMappedChanged()
{
    update();
}

/*!
 * \internal
 */
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

/*!
 * \internal
 */
void QWaylandQuickItem::updateSize()
{
    Q_D(QWaylandQuickItem);
    if (d->sizeFollowsSurface && surface()) {
        setSize(surface()->size());
    }
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandQuickItem::focusOnClick
 *
 * This property specifies whether the WaylandQuickItem should take focus when
 * it is clicked.
 *
 * The default is true.
 */

/*!
 * \property QWaylandQuickItem::focusOnClick
 *
 * This property specifies whether the QWaylandQuickItem should take focus when
 * it is clicked.
 *
 * The default is true.
 */
bool QWaylandQuickItem::focusOnClick() const
{
    Q_D(const QWaylandQuickItem);
    return d->focusOnClick;
}

void QWaylandQuickItem::setFocusOnClick(bool focus)
{
    Q_D(QWaylandQuickItem);
    if (d->focusOnClick == focus)
        return;

    d->focusOnClick = focus;
    emit focusOnClickChanged();
}

/*!
 * Returns true if the input region of this item's surface contains the
 * position given by \a localPosition.
 */
bool QWaylandQuickItem::inputRegionContains(const QPointF &localPosition)
{
    if (QWaylandSurface *s = surface())
        return s->inputRegionContains(localPosition.toPoint());
    return false;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandQuickItem::sizeFollowsSurface
 *
 * This property specifies whether the size of the item should always match
 * the size of its surface.
 *
 * The default is true.
 */

/*!
 * \property QWaylandQuickItem::sizeFollowsSurface
 *
 * This property specifies whether the size of the item should always match
 * the size of its surface.
 *
 * The default is true.
 */
bool QWaylandQuickItem::sizeFollowsSurface() const
{
    Q_D(const QWaylandQuickItem);
    return d->sizeFollowsSurface;
}

void QWaylandQuickItem::setSizeFollowsSurface(bool sizeFollowsSurface)
{
    Q_D(QWaylandQuickItem);
    if (d->sizeFollowsSurface == sizeFollowsSurface)
        return;
    d->sizeFollowsSurface = sizeFollowsSurface;
    emit sizeFollowsSurfaceChanged();
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
    Q_D(const QWaylandQuickItem);
    return d->paintEnabled;
}

void QWaylandQuickItem::setPaintEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    d->paintEnabled = enabled;
    update();
}

bool QWaylandQuickItem::touchEventsEnabled() const
{
    Q_D(const QWaylandQuickItem);
    return d->touchEventsEnabled;
}

void QWaylandQuickItem::updateBuffer(bool hasBuffer)
{
    Q_D(QWaylandQuickItem);
    Q_UNUSED(hasBuffer);
    if (d->origin != surface()->origin()) {
        d->origin = surface()->origin();
        emit originChanged();
    }
}

void QWaylandQuickItem::updateWindow()
{
    Q_D(QWaylandQuickItem);
    if (d->connectedWindow) {
        disconnect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync);
    }

    d->connectedWindow = window();

    if (d->connectedWindow) {
        connect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync, Qt::DirectConnection);
    }

    if (compositor() && d->connectedWindow) {
        QWaylandOutput *output = compositor()->outputFor(d->connectedWindow);
        Q_ASSERT(output);
        d->view->setOutput(output);
    }
}

void QWaylandQuickItem::beforeSync()
{
    Q_D(QWaylandQuickItem);
    if (d->view->advance()) {
        d->newTexture = true;
        update();
    }
}

QSGNode *QWaylandQuickItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    Q_D(QWaylandQuickItem);
    bool mapped = (surface() && surface()->isMapped() && d->view->currentBuffer().hasBuffer())
        || (d->view->isBufferLocked() && d->provider);

    if (!mapped || !d->paintEnabled) {
        delete oldNode;
        return 0;
    }

    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node)
        node = new QSGSimpleTextureNode();

    if (!d->provider)
        d->provider = new QWaylandSurfaceTextureProvider();

    if (d->newTexture) {
        d->newTexture = false;
        d->provider->setBufferRef(this, d->view->currentBuffer());
        node->setTexture(d->provider->texture());
    }

    d->provider->setSmooth(smooth());

    if (d->provider->invertY()) {
            node->setRect(0, height(), width(), -height());
    } else {
            node->setRect(0, 0, width(), height());
    }

    return node;
}

void QWaylandQuickItem::setTouchEventsEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    if (d->touchEventsEnabled != enabled) {
        d->touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

bool QWaylandQuickItem::inputEventsEnabled() const
{
    Q_D(const QWaylandQuickItem);
    return d->inputEventsEnabled;
}

void QWaylandQuickItem::setInputEventsEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    if (d->inputEventsEnabled != enabled) {
        d->inputEventsEnabled = enabled;
        setAcceptHoverEvents(enabled);
        emit inputEventsEnabledChanged();
    }
}

void QWaylandQuickItem::lower()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *bottom = parent->childItems().first();
    if (this != bottom)
        stackBefore(bottom);
}

void QWaylandQuickItem::raise()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *top = parent->childItems().last();
    if (this != top)
        stackAfter(top);
}

QT_END_NAMESPACE
