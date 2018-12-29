/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandquickshellintegration.h"

/*!
 * \class QWaylandQuickShellIntegration
 * \inmodule QtWaylandCompositor
 * \since 5.14
 * \brief Provides support for shell surface integration with QtQuick
 *
 * Shell surface implementations should inherit from this class in order to provide
 * an integration between the shell surface and QtQuick.
 *
 * \sa QWaylandShellSurface
 * \sa QWaylandShellSurfaceItem
 */

QWaylandQuickShellIntegration::QWaylandQuickShellIntegration(QObject *parent)
    : QObject(parent)
{
}

/*!
 * This method can be reimplemented in a subclass to receive touch events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::touchEvent()
 */
bool QWaylandQuickShellIntegration::touchEvent(QTouchEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive hover-enter events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Hover events are only provided if \l {QWaylandQuickShellSurfaceItem::} {acceptHoverEvents()}
 * is \a true.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::hoverEnterEvent()
 */
bool QWaylandQuickShellIntegration::hoverEnterEvent(QHoverEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive hover-leave events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Hover events are only provided if \l {QWaylandQuickShellSurfaceItem::} {acceptHoverEvents()}
 * is \a true.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::hoverLeaveEvent()
 */
bool QWaylandQuickShellIntegration::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive hover-move events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Hover events are only provided if \l {QWaylandQuickShellSurfaceItem::} {acceptHoverEvents()}
 * is \a true.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::hoverMoveEvent()
 */
bool QWaylandQuickShellIntegration::hoverMoveEvent(QHoverEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive key press events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::keyPressEvent()
 */
bool QWaylandQuickShellIntegration::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive key release events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::keyReleaseEvent()
 */
bool QWaylandQuickShellIntegration::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive mouse double click events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::mouseDoubleClickEvent()
 */
bool QWaylandQuickShellIntegration::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive mouse move events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::mouseMoveEvent()
 */
bool QWaylandQuickShellIntegration::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive mouse press events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::mousePressEvent()
 */
bool QWaylandQuickShellIntegration::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 * This method can be reimplemented in a subclass to receive mouse release events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::mouseReleaseEvent()
 */
bool QWaylandQuickShellIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return false;
}

#if QT_CONFIG(wheelevent)
/*!
 * This method can be reimplemented in a subclass to receive wheel events
 * for a shell surface.
 *
 * The event information is provided by the \a event parameter.
 *
 * Return \a false if you want QWaylandQuickShellSurfaceItem to handle
 * the event.
 *
 * \sa QWaylandQuickShellSurfaceItem::wheelEvent()
 */
bool QWaylandQuickShellIntegration::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
    return false;
}
#endif
