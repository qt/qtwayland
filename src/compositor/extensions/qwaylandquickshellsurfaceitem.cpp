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

#include "qwaylandquickshellsurfaceitem.h"
#include "qwaylandquickshellsurfaceitem_p.h"

#include <QtWaylandCompositor/QWaylandShellSurface>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype ShellSurfaceItem
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief An item representing a WlShellSurface.
 *
 * This type is used to render wl_shell or xdg_shell surfaces as part of a Qt Quick
 * scene. It handles moving and resizing triggered by clicking on the window decorations.
 */

/*!
 * \class QWaylandQuickShellSurfaceItem
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief A Qt Quick item for QWaylandShellSurface.
 *
 * This class is used to render wl_shell or xdg_shell surfaces as part of a Qt Quick
 * scene. It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa QWaylandQuickItem
 */

/*!
 * Constructs a QWaylandQuickWlShellSurfaceItem with the given \a parent.
 */
QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QWaylandQuickShellSurfaceItemPrivate(), parent)
{
}

/*!
 * \internal
 */
QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

/*!
 * \qmlproperty object QtWaylandCompositor::ShellSurfaceItem::shellSurface
 *
 * This property holds the shell surface rendered by this ShellSurfaceItem.
 * It may either be an XdgSurface or a WlShellSurface depending on which
 * shell protocol is in use.
 */

/*!
 * \property QWaylandQuickShellSurfaceItem::shellSurface
 *
 * This property holds the shell surface rendered by this
 * QWaylandQuickShellSurfaceItem. It may either be a QWaylandXdgSurface or a
 * QWaylandWlShellSurface depending on which shell protocol is in use.
 *
 */
QWaylandShellSurface *QWaylandQuickShellSurfaceItem::shellSurface() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->m_shellSurface;
}

void QWaylandQuickShellSurfaceItem::setShellSurface(QWaylandShellSurface *shellSurface)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->m_shellSurface == shellSurface)
        return;

    d->m_shellSurface = shellSurface;

    d->m_shellIntegration = shellSurface->createIntegration(this);
    emit shellSurfaceChanged();
}

/*!
 * \property QWaylandQuickShellSurfaceItem::moveItem
 *
 * This property holds the move item for this QWaylandQuickShellSurfaceItem.
 */
QQuickItem *QWaylandQuickShellSurfaceItem::moveItem() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->m_moveItem ? d->m_moveItem : const_cast<QWaylandQuickShellSurfaceItem *>(this);
}

void QWaylandQuickShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    moveItem = moveItem ? moveItem : this;
    if (this->moveItem() == moveItem)
        return;
    d->m_moveItem = moveItem;
    moveItemChanged();
}

void QWaylandQuickShellSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (!d->m_shellIntegration->mouseMoveEvent(event))
        QWaylandQuickItem::mouseMoveEvent(event);
}

void QWaylandQuickShellSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (!d->m_shellIntegration->mouseReleaseEvent(event))
        QWaylandQuickItem::mouseReleaseEvent(event);
}

QT_END_NAMESPACE
