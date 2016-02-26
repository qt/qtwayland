/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwaylandquickxdgsurfaceitem.h"
#include "qwaylandquickxdgsurfaceitem_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandInputDevice>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype XdgSurfaceItem
 * \inqmlmodule QtWayland.Compositor
 * \brief An item representing a XdgSurface.
 *
 * This type can be used to render xdg surfaces as part of a Qt Quick scene.
 * It handles moving and resizing triggered by clicking on the window decorations.
 */

/*!
 * \class QWaylandQuickXdgSurfaceItem
 * \inmodule QtWaylandCompositor
 * \brief A Qt Quick item for QWaylandXdgSurface.
 *
 * This class can be used to create Qt Quick items representing xdg surfaces.
 * It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa QWaylandQuickItem
 */

/*!
 * Constructs a QWaylandQuickXdgSurfaceItem with the given \a parent.
 */
QWaylandQuickXdgSurfaceItem::QWaylandQuickXdgSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QWaylandQuickXdgSurfaceItemPrivate(), parent)
{
}

/*!
 * \internal
 */
QWaylandQuickXdgSurfaceItem::QWaylandQuickXdgSurfaceItem(QWaylandQuickXdgSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

/*!
 * \qmlproperty object QtWaylandCompositor::XdgSurfaceItem::xdgSurface
 *
 * This property holds the xdg surface rendered by this XdgSurfaceItem.
 */

/*!
 * \property QWaylandQuickXdgSurfaceItem::xdgSurface
 *
 * This property holds the xdg surface rendered by this QWaylandQuickXdgSurfaceItem.
 */
QWaylandXdgSurface *QWaylandQuickXdgSurfaceItem::xdgSurface() const
{
    Q_D(const QWaylandQuickXdgSurfaceItem);
    return d->xdgSurface;
}

void QWaylandQuickXdgSurfaceItem::setXdgSurface(QWaylandXdgSurface *xdgSurface)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (xdgSurface == d->xdgSurface)
        return;

    if (d->xdgSurface) {
        disconnect(d->xdgSurface, &QWaylandXdgSurface::startMove, this, &QWaylandQuickXdgSurfaceItem::handleStartMove);
        disconnect(d->xdgSurface, &QWaylandXdgSurface::startResize, this, &QWaylandQuickXdgSurfaceItem::handleStartResize);
        disconnect(d->xdgSurface, &QWaylandXdgSurface::setMaximized, this, &QWaylandQuickXdgSurfaceItem::handleSetMaximized);
        disconnect(d->xdgSurface, &QWaylandXdgSurface::unsetMaximized, this, &QWaylandQuickXdgSurfaceItem::handleUnsetMaximized);
        disconnect(d->xdgSurface, &QWaylandXdgSurface::maximizedChanged, this, &QWaylandQuickXdgSurfaceItem::handleMaximizedChanged);
        disconnect(d->xdgSurface, &QWaylandXdgSurface::activatedChanged, this, &QWaylandQuickXdgSurfaceItem::handleActivatedChanged);
    }
    d->xdgSurface = xdgSurface;
    if (d->xdgSurface) {
        connect(d->xdgSurface, &QWaylandXdgSurface::startMove, this, &QWaylandQuickXdgSurfaceItem::handleStartMove);
        connect(d->xdgSurface, &QWaylandXdgSurface::startResize, this, &QWaylandQuickXdgSurfaceItem::handleStartResize);
        connect(d->xdgSurface, &QWaylandXdgSurface::setMaximized, this, &QWaylandQuickXdgSurfaceItem::handleSetMaximized);
        connect(d->xdgSurface, &QWaylandXdgSurface::unsetMaximized, this, &QWaylandQuickXdgSurfaceItem::handleUnsetMaximized);
        connect(d->xdgSurface, &QWaylandXdgSurface::maximizedChanged, this, &QWaylandQuickXdgSurfaceItem::handleMaximizedChanged);
        connect(d->xdgSurface, &QWaylandXdgSurface::activatedChanged, this, &QWaylandQuickXdgSurfaceItem::handleActivatedChanged);
    }
    setSurface(xdgSurface ? xdgSurface->surface() : nullptr);
    emit xdgSurfaceChanged();
}

/*!
 * \internal
 * \property QWaylandQuickXdgSurfaceItem::moveItem
 *
 * This property holds the move item for this QWaylandQuickXdgSurfaceItem.
 */
QQuickItem *QWaylandQuickXdgSurfaceItem::moveItem() const
{
    Q_D(const QWaylandQuickXdgSurfaceItem);
    return d->moveItem;
}

void QWaylandQuickXdgSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (d->moveItem == moveItem)
        return;
    d->moveItem = moveItem;
    moveItemChanged();
}

/*!
 * \internal
 */
void QWaylandQuickXdgSurfaceItem::handleStartMove(QWaylandInputDevice *inputDevice)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    d->grabberState = QWaylandQuickXdgSurfaceItemPrivate::MoveState;
    d->moveState.inputDevice = inputDevice;
    d->moveState.initialized = false;
}

/*!
 * \internal
 */
void QWaylandQuickXdgSurfaceItem::handleStartResize(QWaylandInputDevice *inputDevice, QWaylandXdgSurface::ResizeEdge edges)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    d->grabberState = QWaylandQuickXdgSurfaceItemPrivate::ResizeState;
    d->resizeState.inputDevice = inputDevice;
    d->resizeState.resizeEdges = edges;
    d->resizeState.initialWindowSize = xdgSurface()->windowGeometry().size();
    d->resizeState.initialPosition = position();
    d->resizeState.initialSurfaceSize = surface()->size();
    d->resizeState.initialized = false;
}

void QWaylandQuickXdgSurfaceItem::handleSetMaximized()
{
    Q_D(QWaylandQuickXdgSurfaceItem);

    d->maximizeState.initialWindowSize = xdgSurface()->windowGeometry().size();
    d->maximizeState.initialPosition = position();

    QWaylandOutput *output = compositor()->outputs().first();
    xdgSurface()->requestMaximized(output->geometry().size());
}

void QWaylandQuickXdgSurfaceItem::handleUnsetMaximized()
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    xdgSurface()->requestUnMaximized(d->maximizeState.initialWindowSize);
}

void QWaylandQuickXdgSurfaceItem::handleMaximizedChanged()
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (xdgSurface()->maximized()) {
        QWaylandOutput *output = compositor()->outputs().first();
        setPosition(output->geometry().topLeft());
    } else {
        setPosition(d->maximizeState.initialPosition);
    }
}

void QWaylandQuickXdgSurfaceItem::handleActivatedChanged()
{
    if (xdgSurface()->activated())
        raise();
}

void QWaylandQuickXdgSurfaceItem::handleSurfaceSizeChanged()
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (d->grabberState == QWaylandQuickXdgSurfaceItemPrivate::ResizeState) {
        float x = d->resizeState.initialPosition.x();
        float y = d->resizeState.initialPosition.y();
        if (d->resizeState.resizeEdges & QWaylandXdgSurface::ResizeEdge::TopEdge)
            y += d->resizeState.initialSurfaceSize.height() - surface()->size().height();

        if (d->resizeState.resizeEdges & QWaylandXdgSurface::ResizeEdge::LeftEdge)
            x += d->resizeState.initialSurfaceSize.width() - surface()->size().width();
        setPosition(QPoint(x,y));
    }
}

/*!
 * \internal
 */
void QWaylandQuickXdgSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (d->grabberState == QWaylandQuickXdgSurfaceItemPrivate::ResizeState) {
        Q_ASSERT(d->resizeState.inputDevice == compositor()->inputDeviceFor(event));
        if (!d->resizeState.initialized) {
            d->resizeState.initialMousePos = event->windowPos();
            d->resizeState.initialized = true;
            return;
        }
        QPointF delta = event->windowPos() - d->resizeState.initialMousePos;
        QSize newSize = xdgSurface()->sizeForResize(d->resizeState.initialWindowSize, delta, d->resizeState.resizeEdges);
        xdgSurface()->requestResizing(newSize);
    } else if (d->grabberState == QWaylandQuickXdgSurfaceItemPrivate::MoveState) {
        Q_ASSERT(d->moveState.inputDevice == compositor()->inputDeviceFor(event));
        QQuickItem *moveItem = d->moveItem ? d->moveItem : this;
        if (!d->moveState.initialized) {
            d->moveState.initialOffset = moveItem->mapFromItem(Q_NULLPTR, event->windowPos());
            d->moveState.initialized = true;
            return;
        }
        if (!moveItem->parentItem())
            return;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(Q_NULLPTR, event->windowPos());
        moveItem->setPosition(parentPos - d->moveState.initialOffset);
    } else {
        QWaylandQuickItem::mouseMoveEvent(event);
    }
}

/*!
 * \internal
 */
void QWaylandQuickXdgSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickXdgSurfaceItem);
    if (d->grabberState == QWaylandQuickXdgSurfaceItemPrivate::ResizeState) {
        QPointF delta = event->windowPos() - d->resizeState.initialMousePos;
        QSize newSize = xdgSurface()->sizeForResize(d->resizeState.initialWindowSize, delta, d->resizeState.resizeEdges);
        xdgSurface()->requestUnMaximized(newSize);
        d->grabberState = QWaylandQuickXdgSurfaceItemPrivate::DefaultState;
        return;
    } else if (d->grabberState == QWaylandQuickXdgSurfaceItemPrivate::MoveState) {
        d->grabberState = QWaylandQuickXdgSurfaceItemPrivate::DefaultState;
        return;
    }
    QWaylandQuickItem::mouseReleaseEvent(event);
}

/*!
 * \internal
 */
void QWaylandQuickXdgSurfaceItem::surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    if (oldSurface)
        disconnect(oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickXdgSurfaceItem::handleSurfaceSizeChanged);

    if (newSurface)
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickXdgSurfaceItem::handleSurfaceSizeChanged);
}

QT_END_NAMESPACE
