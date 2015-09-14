/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
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

#include "qwaylandquickshellsurfaceitem.h"
#include "qwaylandquickshellsurfaceitem_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandInputDevice>

QT_BEGIN_NAMESPACE

QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QWaylandQuickShellSurfaceItemPrivate(), parent)
{
}

QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

QWaylandShellSurface *QWaylandQuickShellSurfaceItem::shellSurface() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->shellSurface;
}

void QWaylandQuickShellSurfaceItem::setShellSurface(QWaylandShellSurface *shellSurface)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (shellSurface == d->shellSurface)
        return;

    if (d->shellSurface) {
        disconnect(d->shellSurface, &QWaylandShellSurface::startMove, this, &QWaylandQuickShellSurfaceItem::handleStartMove);
        disconnect(d->shellSurface, &QWaylandShellSurface::startResize, this, &QWaylandQuickShellSurfaceItem::handleStartResize);
    }
    d->shellSurface = shellSurface;
    if (d->shellSurface) {
        connect(d->shellSurface, &QWaylandShellSurface::startMove, this, &QWaylandQuickShellSurfaceItem::handleStartMove);
        connect(d->shellSurface, &QWaylandShellSurface::startResize, this, &QWaylandQuickShellSurfaceItem::handleStartResize);
    }
    emit shellSurfaceChanged();
}

QQuickItem *QWaylandQuickShellSurfaceItem::moveItem() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->moveItem;
}

void QWaylandQuickShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->moveItem == moveItem)
        return;
    d->moveItem = moveItem;
    moveItemChanged();
}

void QWaylandQuickShellSurfaceItem::handleStartMove(QWaylandInputDevice *inputDevice)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    d->grabberState = QWaylandQuickShellSurfaceItemPrivate::MoveState;
    d->moveState.inputDevice = inputDevice;
    d->moveState.initialized = false;
}

void QWaylandQuickShellSurfaceItem::handleStartResize(QWaylandInputDevice *inputDevice, QWaylandShellSurface::ResizeEdge edges)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    d->grabberState = QWaylandQuickShellSurfaceItemPrivate::ResizeState;
    d->resizeState.inputDevice = inputDevice;
    d->resizeState.resizeEdges = edges;
    d->resizeState.initialSize = surface()->size();
    d->resizeState.initialized = false;
}

void QWaylandQuickShellSurfaceItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    QQuickItem *moveItem = d->moveItem ? d->moveItem : this;
    moveItem->setPosition(moveItem->position() + offset);
}

void QWaylandQuickShellSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->grabberState == QWaylandQuickShellSurfaceItemPrivate::ResizeState) {
        Q_ASSERT(d->resizeState.inputDevice == compositor()->inputDeviceFor(event));
        if (!d->resizeState.initialized) {
            d->resizeState.initialMousePos = event->windowPos();
            d->resizeState.initialized = true;
            return;
        }
        QPointF delta = event->windowPos() - d->resizeState.initialMousePos;
        QSize newSize = shellSurface()->sizeForResize(d->resizeState.initialSize, delta, d->resizeState.resizeEdges);
        shellSurface()->sendConfigure(newSize, d->resizeState.resizeEdges);
    } else if (d->grabberState == QWaylandQuickShellSurfaceItemPrivate::MoveState) {
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

void QWaylandQuickShellSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->grabberState != QWaylandQuickShellSurfaceItemPrivate::DefaultState) {
        d->grabberState = QWaylandQuickShellSurfaceItemPrivate::DefaultState;
        return;
    }
    QWaylandQuickItem::mouseReleaseEvent(event);
}

void QWaylandQuickShellSurfaceItem::surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    if (oldSurface)
        disconnect(oldSurface, &QWaylandSurface::offsetForNextFrame, this, &QWaylandQuickShellSurfaceItem::adjustOffsetForNextFrame);

    if (newSurface)
        connect(newSurface, &QWaylandSurface::offsetForNextFrame, this, &QWaylandQuickShellSurfaceItem::adjustOffsetForNextFrame);
}

void QWaylandQuickShellSurfaceItem::componentComplete()
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (!d->shellSurface)
        setShellSurface(new QWaylandShellSurface());

    QWaylandQuickItem::componentComplete();
}

QT_END_NAMESPACE
