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
#include <QGuiApplication>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype WlShellSurfaceItem
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \preliminary
 * \brief An item representing a WlShellSurface.
 *
 * This type can be used to render wl_shell_surfaces as part of a Qt Quick scene.
 * It handles moving and resizing triggered by clicking on the window decorations.
 */

/*!
 * \class QWaylandQuickWlShellSurfaceItem
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \preliminary
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
        disconnect(d->shellSurface, &QWaylandWlShellSurface::setPopup, this, &QWaylandQuickWlShellSurfaceItem::handleSetPopup);
        disconnect(d->shellSurface, &QWaylandWlShellSurface::destroyed, this, &QWaylandQuickWlShellSurfaceItem::handleShellSurfaceDestroyed);
    }
    d->shellSurface = shellSurface;
    if (d->shellSurface) {
        connect(d->shellSurface, &QWaylandWlShellSurface::startMove, this, &QWaylandQuickWlShellSurfaceItem::handleStartMove);
        connect(d->shellSurface, &QWaylandWlShellSurface::startResize, this, &QWaylandQuickWlShellSurfaceItem::handleStartResize);
        connect(d->shellSurface, &QWaylandWlShellSurface::setPopup, this, &QWaylandQuickWlShellSurfaceItem::handleSetPopup);
        connect(d->shellSurface, &QWaylandWlShellSurface::destroyed, this, &QWaylandQuickWlShellSurfaceItem::handleShellSurfaceDestroyed);
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
    d->resizeState.initialSize = surface()->size() / d->scaleFactor();
    d->resizeState.initialized = false;
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::handleSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(inputDevice);
    Q_D(QWaylandQuickWlShellSurfaceItem);

    QWaylandQuickWlShellSurfaceItem* parentItem = qobject_cast<QWaylandQuickWlShellSurfaceItem*>(parent->views().first()->renderObject());
    if (parentItem) {
        // Clear all the transforms for this ShellSurfaceItem. They are not
        // applicable when the item becomes a child to a surface that has its
        // own transforms. Otherwise the transforms would be applied twice.
        QQmlListProperty<QQuickTransform> t = transform();
        t.clear(&t);
        setRotation(0);
        setScale(1.0);
        setX(relativeToParent.x());
        setY(relativeToParent.y());
        setParentItem(parentItem);
    }

    d->setIsPopup(true);
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::handleShellSurfaceDestroyed() {
    Q_D(QWaylandQuickWlShellSurfaceItem);
    d->setIsPopup(false);
    d->shellSurface = NULL;
}

void QWaylandQuickWlShellSurfaceItem::handleSurfaceUnmapped()
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (!d->shellSurface || !d->shellSurface->surface()->size().isEmpty())
        return;
    d->setIsPopup(false);
}

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItem::adjustOffsetForNextFrame(const QPointF &offset)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    QQuickItem *moveItem = d->moveItem ? d->moveItem : this;
    moveItem->setPosition(moveItem->position() + offset * d->scaleFactor());
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
        QPointF delta = (event->windowPos() - d->resizeState.initialMousePos) / d->scaleFactor();
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

QVector<QWaylandWlShellSurface*> QWaylandQuickWlShellSurfaceItemPrivate::popupShellSurfaces;
bool QWaylandQuickWlShellSurfaceItemPrivate::eventFilterInstalled = false;
bool QWaylandQuickWlShellSurfaceItemPrivate::waitForRelease = false;

/*!
 * \internal
 */
void QWaylandQuickWlShellSurfaceItemPrivate::closePopups()
{
    if (!popupShellSurfaces.isEmpty()) {
        Q_FOREACH (QWaylandWlShellSurface* shellSurface, popupShellSurfaces) {
            shellSurface->sendPopupDone();
        }
        popupShellSurfaces.clear();
    }
}

bool QWaylandQuickWlShellSurfaceItem::eventFilter(QObject *receiver, QEvent *e)
{
    Q_D(QWaylandQuickWlShellSurfaceItem);
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        QQuickItem *item = qobject_cast<QQuickItem*>(receiver);
        if (!item)
            return false;

        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        QWaylandQuickWlShellSurfaceItem *shellSurfaceItem = qobject_cast<QWaylandQuickWlShellSurfaceItem*>(item);
        bool press = event->type() == QEvent::MouseButtonPress;
        bool finalRelease = (event->type() == QEvent::MouseButtonRelease) && (event->buttons() == Qt::NoButton);
        bool popupClient = shellSurfaceItem && shellSurfaceItem->shellSurface()->surface()->client() == shellSurface()->surface()->client();

        if (d->waitForRelease) {
            // We are eating events until all mouse buttons are released
            if (finalRelease) {
                d->waitForRelease = false;
                d->setFilterEnabled(false);
            }
            return true;
        }

        if (press && !popupClient) {
            // The user clicked outside the active popup's client. The popups should
            // be closed, but the event filter will stay to catch the release-
            // event before removing itself.
            d->waitForRelease = true;
            d->closePopups();
            return true;
        } else if (press) {
            // There is a surface belonging to this client at this coordinate, so we can
            // remove the event filter and let the normal event handler handle
            // this event.
            d->setFilterEnabled(false);
        }
    }

    return false;
}

void QWaylandQuickWlShellSurfaceItemPrivate::setIsPopup(bool popup)
{
    Q_Q(QWaylandQuickWlShellSurfaceItem);
    isPopup = popup;
    if (popup) {
        if (!eventFilterInstalled)
           setFilterEnabled(true);

        if (!popupShellSurfaces.contains(shellSurface)) {
            popupShellSurfaces.append(shellSurface);
            QObject::connect(shellSurface->surface(), &QWaylandSurface::mappedChanged,
                             q, &QWaylandQuickWlShellSurfaceItem::handleSurfaceUnmapped);
        }
    } else {
        if (shellSurface) {
            popupShellSurfaces.removeOne(shellSurface);
            QObject::disconnect(shellSurface->surface(), &QWaylandSurface::mappedChanged,
                             q, &QWaylandQuickWlShellSurfaceItem::handleSurfaceUnmapped);
        }
        if (!waitForRelease && eventFilterInstalled && popupShellSurfaces.isEmpty())
            setFilterEnabled(false);
    }
}

void QWaylandQuickWlShellSurfaceItemPrivate::setFilterEnabled(bool enabled)
{
    Q_Q(QWaylandQuickWlShellSurfaceItem);
    static QPointer<QObject> filter;

    if (enabled && filter.isNull()) {
        qGuiApp->installEventFilter(q);
        filter = q;
    } else if (!enabled && !filter.isNull()){
        qGuiApp->removeEventFilter(filter);
        filter = nullptr;
    }
    eventFilterInstalled = enabled;
}

QT_END_NAMESPACE
