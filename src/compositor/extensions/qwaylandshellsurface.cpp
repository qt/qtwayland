/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <QtWaylandCompositor/QWaylandShellSurface>

/*!
 * \qmltype ShellSurface
 * \instantiates QWaylandShellSurface
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief Provides a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the WaylandSurface should map onto the screen.
 *
 * Note: Even though this type contains a very limited API, the properties and signals of the
 * implementations are named consistently. For example, if you're only using desktop shell
 * extensions in your compositor, it's safe to access properties such as title, maximized, etc.
 * directly on the ShellSurface. See the various implementations for additional properties and
 * signals.
 *
 * \sa WaylandSurface, ShellSurfaceItem, WlShellSurface, IviSurface
 */

/*!
 * \class QWaylandShellSurface
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandShellSurface class is a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the QWaylandSurface should map onto the screen.
 *
 * \sa QWaylandSurface, QWaylandWlShellSurface, QWaylandIviSurface
 */

#if QT_CONFIG(wayland_compositor_quick)
/*!
 * \fn QWaylandQuickShellIntegration *QWaylandShellSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
 *
 * Creates a QWaylandQuickShellIntegration for this QWaylandQuickShellSurface. It's called
 * automatically when \a {item}'s \l {QWaylandQuickShellSurfaceItem::}{shellSurface} is assigned.
 *
 * \sa QWaylandQuickShellSurfaceItem
 */
#endif

/*!
 * \qmlproperty enum QtWaylandCompositor::ShellSurface::windowType
 *
 * This property holds the window type of the ShellSurface.
 */

/*!
 * \property QWaylandShellSurface::windowType
 *
 * This property holds the window type of the QWaylandShellSurface.
 */

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

#include "moc_qwaylandshellsurface.cpp"
