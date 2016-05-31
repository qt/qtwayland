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

#include "qwaylandwlshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandInputDevice>

QT_BEGIN_NAMESPACE

namespace QtWayland {

WlShellIntegration::WlShellIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<QWaylandWlShellSurface *>(item->shellSurface()))
    , grabberState(GrabberState::Default)
    , isPopup(false)
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &QWaylandWlShellSurface::startMove, this, &WlShellIntegration::handleStartMove);
    connect(m_shellSurface, &QWaylandWlShellSurface::startResize, this, &WlShellIntegration::handleStartResize);
    connect(m_shellSurface->surface(), &QWaylandSurface::offsetForNextFrame, this, &WlShellIntegration::adjustOffsetForNextFrame);
    connect(m_shellSurface, &QWaylandWlShellSurface::setPopup, this, &WlShellIntegration::handleSetPopup);
    connect(m_shellSurface, &QWaylandWlShellSurface::destroyed, this, &WlShellIntegration::handleShellSurfaceDestroyed);
}

void WlShellIntegration::handleStartMove(QWaylandInputDevice *inputDevice)
{
    grabberState = GrabberState::Move;
    moveState.inputDevice = inputDevice;
    moveState.initialized = false;
}

void WlShellIntegration::handleStartResize(QWaylandInputDevice *inputDevice, QWaylandWlShellSurface::ResizeEdge edges)
{
    grabberState = GrabberState::Resize;
    resizeState.inputDevice = inputDevice;
    resizeState.resizeEdges = edges;
    float scaleFactor = m_item->view()->output()->scaleFactor();
    resizeState.initialSize = m_shellSurface->surface()->size() / scaleFactor;
    resizeState.initialized = false;
}

void WlShellIntegration::handleSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(inputDevice);

    QWaylandQuickShellSurfaceItem* parentItem = qobject_cast<QWaylandQuickShellSurfaceItem*>(parent->views().first()->renderObject());
    if (parentItem) {
        // Clear all the transforms for this ShellSurfaceItem. They are not
        // applicable when the item becomes a child to a surface that has its
        // own transforms. Otherwise the transforms would be applied twice.
        QQmlListProperty<QQuickTransform> t = m_item->transform();
        t.clear(&t);
        m_item->setRotation(0);
        m_item->setScale(1.0);
        m_item->setX(relativeToParent.x());
        m_item->setY(relativeToParent.y());
        m_item->setParentItem(parentItem);
    }

    isPopup = true;
    QWaylandQuickShellEventFilter::startFilter(m_shellSurface->surface()->client(), &closePopups);

    if (!popupShellSurfaces.contains(m_shellSurface)) {
        popupShellSurfaces.append(m_shellSurface);
        QObject::connect(m_shellSurface->surface(), &QWaylandSurface::mappedChanged,
                         this, &WlShellIntegration::handleSurfaceUnmapped);
    }
}

void WlShellIntegration::handlePopupClosed()
{
    handlePopupRemoved();
    if (m_shellSurface)
        QObject::disconnect(m_shellSurface->surface(), &QWaylandSurface::mappedChanged,
                            this, &WlShellIntegration::handleSurfaceUnmapped);
}

void WlShellIntegration::handlePopupRemoved()
{
    if (m_shellSurface)
        popupShellSurfaces.removeOne(m_shellSurface);
    if (popupShellSurfaces.isEmpty())
        QWaylandQuickShellEventFilter::cancelFilter();
    isPopup = false;
}


void WlShellIntegration::handleShellSurfaceDestroyed()
{
    if (isPopup)
        handlePopupRemoved();
    m_shellSurface = nullptr;
}

void WlShellIntegration::handleSurfaceUnmapped()
{
    if (!m_shellSurface || !m_shellSurface->surface()->size().isEmpty())
        return;
    handlePopupClosed();
}

void WlShellIntegration::adjustOffsetForNextFrame(const QPointF &offset)
{
    float scaleFactor = m_item->view()->output()->scaleFactor();
    QQuickItem *moveItem = m_item->moveItem();
    moveItem->setPosition(moveItem->position() + offset * scaleFactor);
}

bool WlShellIntegration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        Q_ASSERT(resizeState.inputDevice == m_item->compositor()->inputDeviceFor(event));
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        float scaleFactor = m_item->view()->output()->scaleFactor();
        QPointF delta = (event->windowPos() - resizeState.initialMousePos) / scaleFactor;
        QSize newSize = m_shellSurface->sizeForResize(resizeState.initialSize, delta, resizeState.resizeEdges);
        m_shellSurface->sendConfigure(newSize, resizeState.resizeEdges);
    } else if (grabberState == GrabberState::Move) {
        Q_ASSERT(moveState.inputDevice == m_item->compositor()->inputDeviceFor(event));
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->windowPos());
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->windowPos());
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }
    return false;
}

bool WlShellIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

QVector<QWaylandWlShellSurface*> WlShellIntegration::popupShellSurfaces;

void WlShellIntegration::closePopups()
{
    if (!popupShellSurfaces.isEmpty()) {
        Q_FOREACH (QWaylandWlShellSurface* shellSurface, popupShellSurfaces) {
            shellSurface->sendPopupDone();
        }
        popupShellSurfaces.clear();
    }
}

}

QT_END_NAMESPACE
