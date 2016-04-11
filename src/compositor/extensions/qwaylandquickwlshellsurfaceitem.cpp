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

#include "qwaylandquickwlshellsurfaceitem.h"
#include "qwaylandquickwlshellsurfaceitem_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandInputDevice>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype WlShellSurfaceItem
 * \inqmlmodule QtWayland.Compositor
 * \brief An item representing a WlShellSurface.
 *
 * This type can be used to render wl_shell_surfaces as part of a Qt Quick scene.
 * It handles moving and resizing triggered by clicking on the window decorations.
 */

/*!
 * \class QWaylandQuickWlShellSurfaceItem
 * \inmodule QtWaylandCompositor
 * \brief A Qt Quick item for QWaylandWlShellSurface.
 *
 * This class can be used to create Qt Quick items representing wl_shell_surfaces.
 * It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa QWaylandQuickItem
 */

/*!
 * Constructs a QWaylandQuickWlShellSurfaceItem with the given \a parent.
 */
QWaylandQuickWlShellSurfaceItem::QWaylandQuickWlShellSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QWaylandQuickWlShellSurfaceItemPrivate(), parent)
{
}

/*!
 * \internal
 */
QWaylandQuickWlShellSurfaceItem::QWaylandQuickWlShellSurfaceItem(QWaylandQuickWlShellSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

/*!
 * \qmlproperty object QtWaylandCompositor::WlShellSurfaceItem::shellSurface
 *
 * This property holds the wl_shell_surface rendered by this WlShellSurfaceItem.
 */

/*!
 * \property QWaylandQuickWlShellSurfaceItem::shellSurface
 *
 * This property holds the wl_shell_surface rendered by this QWaylandQuickWlShellSurfaceItem.
 */
QWaylandWlShellSurface *QWaylandQuickWlShellSurfaceItem::shellSurface() const
{
    Q_D(const QWaylandQuickWlShellSurfaceItem);
    return d->shellSurface;
}

void QWaylandQuickWlShellSurfaceItem::setShellSurface(QWaylandWlShellSurface *shellSurface)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (shellSurface == d->shellSurface)
        return;

    if (d->shellSurface) {
        disconnect(d->shellSurface, &QWaylandWlShellSurface::startMove, this, &QWaylandQuickWlShellSurfaceItem::handleStartMove);
        disconnect(d->shellSurface, &QWaylandWlShellSurface::startResize, this, &QWaylandQuickWlShellSurfaceItem::handleStartResize);
    }
    d->shellSurface = shellSurface;
    if (d->shellSurface) {
        connect(d->shellSurface, &QWaylandWlShellSurface::startMove, this, &QWaylandQuickWlShellSurfaceItem::handleStartMove);
        connect(d->shellSurface, &QWaylandWlShellSurface::startResize, this, &QWaylandQuickWlShellSurfaceItem::handleStartResize);
    }
    setSurface(shellSurface ? shellSurface->surface() : nullptr);
    emit shellSurfaceChanged();
}

/*!
 * \internal
 * \property QWaylandQuickWlShellSurfaceItem::moveItem
 *
 * This property holds the move item for this QWaylandQuickWlShellSurfaceItem.
 */
QQuickItem *QWaylandQuickWlShellSurfaceItem::moveItem() const
{
    Q_D(const QWaylandQuickWlShellSurfaceItem);
    return d->moveItem;
}

void QWaylandQuickWlShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (d->moveItem == moveItem)
        return;
    d->moveItem = moveItem;
    moveItemChanged();
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::handleStartMove(QWaylandInputDevice *inputDevice)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    d->grabberState = QWaylandQuickWlShellSurfaceItemPrivate::MoveState;
    d->moveState.inputDevice = inputDevice;
    d->moveState.initialized = false;
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::handleStartResize(QWaylandInputDevice *inputDevice, QWaylandWlShellSurface::ResizeEdge edges)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    d->grabberState = QWaylandQuickWlShellSurfaceItemPrivate::ResizeState;
    d->resizeState.inputDevice = inputDevice;
    d->resizeState.resizeEdges = edges;
    d->resizeState.initialSize = surface()->size();
    d->resizeState.initialized = false;
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    QQuickItem *moveItem = d->moveItem ? d->moveItem : this;
    moveItem->setPosition(moveItem->position() + offset);
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (d->grabberState == QWaylandQuickWlShellSurfaceItemPrivate::ResizeState) {
        Q_ASSERT(d->resizeState.inputDevice == compositor()->inputDeviceFor(event));
        if (!d->resizeState.initialized) {
            d->resizeState.initialMousePos = event->windowPos();
            d->resizeState.initialized = true;
            return;
        }
        QPointF delta = event->windowPos() - d->resizeState.initialMousePos;
        QSize newSize = shellSurface()->sizeForResize(d->resizeState.initialSize, delta, d->resizeState.resizeEdges);
        shellSurface()->sendConfigure(newSize, d->resizeState.resizeEdges);
    } else if (d->grabberState == QWaylandQuickWlShellSurfaceItemPrivate::MoveState) {
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
void QWaylandQuickWlShellSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (d->grabberState != QWaylandQuickWlShellSurfaceItemPrivate::DefaultState) {
        d->grabberState = QWaylandQuickWlShellSurfaceItemPrivate::DefaultState;
        return;
    }
    QWaylandQuickItem::mouseReleaseEvent(event);
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    if (oldSurface)
        disconnect(oldSurface, &QWaylandSurface::offsetForNextFrame, this, &QWaylandQuickWlShellSurfaceItem::adjustOffsetForNextFrame);

    if (newSurface)
        connect(newSurface, &QWaylandSurface::offsetForNextFrame, this, &QWaylandQuickWlShellSurfaceItem::adjustOffsetForNextFrame);
}

QT_END_NAMESPACE
