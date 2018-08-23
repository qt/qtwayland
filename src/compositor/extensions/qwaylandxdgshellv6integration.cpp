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

#include "qwaylandxdgshellv6integration_p.h"

#include <QtWaylandCompositor/QWaylandXdgSurfaceV6>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

QT_BEGIN_NAMESPACE

namespace QtWayland {

XdgToplevelV6Integration::XdgToplevelV6Integration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurfaceV6 *>(item->shellSurface()))
    , m_toplevel(m_xdgSurface->toplevel())
    , grabberState(GrabberState::Default)
{
    Q_ASSERT(m_toplevel);

    m_item->setSurface(m_xdgSurface->surface());
    connect(m_toplevel, &QWaylandXdgToplevelV6::startMove, this, &XdgToplevelV6Integration::handleStartMove);
    connect(m_toplevel, &QWaylandXdgToplevelV6::startResize, this, &XdgToplevelV6Integration::handleStartResize);
    connect(m_toplevel, &QWaylandXdgToplevelV6::setMaximized, this, &XdgToplevelV6Integration::handleSetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevelV6::unsetMaximized, this, &XdgToplevelV6Integration::handleUnsetMaximized);
    connect(m_toplevel, &QWaylandXdgToplevelV6::maximizedChanged, this, &XdgToplevelV6Integration::handleMaximizedChanged);
    connect(m_toplevel, &QWaylandXdgToplevelV6::activatedChanged, this, &XdgToplevelV6Integration::handleActivatedChanged);
    connect(m_xdgSurface->surface(), &QWaylandSurface::sizeChanged, this, &XdgToplevelV6Integration::handleSurfaceSizeChanged);
}

bool XdgToplevelV6Integration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        Q_ASSERT(resizeState.seat == m_item->compositor()->seatFor(event));
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        QPointF delta = m_item->mapToSurface(event->windowPos() - resizeState.initialMousePos);
        QSize newSize = m_toplevel->sizeForResize(resizeState.initialWindowSize, delta, resizeState.resizeEdges);
        m_toplevel->sendResizing(newSize);
    } else if (grabberState == GrabberState::Move) {
        Q_ASSERT(moveState.seat == m_item->compositor()->seatFor(event));
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

bool XdgToplevelV6Integration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

void XdgToplevelV6Integration::handleStartMove(QWaylandSeat *seat)
{
    grabberState = GrabberState::Move;
    moveState.seat = seat;
    moveState.initialized = false;
}

void XdgToplevelV6Integration::handleStartResize(QWaylandSeat *seat, Qt::Edges edges)
{
    grabberState = GrabberState::Resize;
    resizeState.seat = seat;
    resizeState.resizeEdges = edges;
    resizeState.initialWindowSize = m_xdgSurface->windowGeometry().size();
    resizeState.initialPosition = m_item->moveItem()->position();
    resizeState.initialSurfaceSize = m_item->surface()->size();
    resizeState.initialized = false;
}

void XdgToplevelV6Integration::handleSetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    maximizeState.initialWindowSize = m_xdgSurface->windowGeometry().size();
    maximizeState.initialPosition = m_item->moveItem()->position();

    QWaylandOutput *output = m_item->view()->output();
    m_toplevel->sendMaximized(output->availableGeometry().size() / output->scaleFactor());
}

void XdgToplevelV6Integration::handleUnsetMaximized()
{
    if (!m_item->view()->isPrimary())
        return;

    m_toplevel->sendUnmaximized(maximizeState.initialWindowSize);
}

void XdgToplevelV6Integration::handleMaximizedChanged()
{
    if (m_toplevel->maximized()) {
        QWaylandOutput *output = m_item->view()->output();
        m_item->moveItem()->setPosition(output->position() + output->availableGeometry().topLeft());
    } else {
        m_item->moveItem()->setPosition(maximizeState.initialPosition);
    }
}

void XdgToplevelV6Integration::handleActivatedChanged()
{
    if (m_toplevel->activated())
        m_item->raise();
}

void XdgToplevelV6Integration::handleSurfaceSizeChanged()
{
    if (grabberState == GrabberState::Resize) {
        qreal x = resizeState.initialPosition.x();
        qreal y = resizeState.initialPosition.y();
        if (resizeState.resizeEdges & Qt::TopEdge)
            y += resizeState.initialSurfaceSize.height() - m_item->surface()->size().height();

        if (resizeState.resizeEdges & Qt::LeftEdge)
            x += resizeState.initialSurfaceSize.width() - m_item->surface()->size().width();
        m_item->moveItem()->setPosition(QPointF(x, y));
    }
}

XdgPopupV6Integration::XdgPopupV6Integration(QWaylandQuickShellSurfaceItem *item)
    : m_item(item)
    , m_xdgSurface(qobject_cast<QWaylandXdgSurfaceV6 *>(item->shellSurface()))
    , m_popup(m_xdgSurface->popup())
{
    Q_ASSERT(m_popup);

    m_item->setSurface(m_xdgSurface->surface());
    handleGeometryChanged();

    connect(m_popup, &QWaylandXdgPopupV6::configuredGeometryChanged, this, &XdgPopupV6Integration::handleGeometryChanged);
}

void XdgPopupV6Integration::handleGeometryChanged()
{
    if (m_item->view()->output()) {
        const QPoint windowOffset = m_popup->parentXdgSurface()->windowGeometry().topLeft();
        const QPoint position = m_popup->unconstrainedPosition() + windowOffset;
        //TODO: positioner size or other size...?
        const float scaleFactor = m_item->view()->output()->scaleFactor();
        //TODO check positioner constraints etc... sliding, flipping
        m_item->moveItem()->setPosition(position * scaleFactor);
    } else {
        qWarning() << "XdgPopupV6Integration popup item without output" << m_item;
    }
}

}

QT_END_NAMESPACE
