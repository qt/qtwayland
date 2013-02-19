/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandshellsurface.h"

#include "qwaylanddisplay.h"
#include "qwaylandwindow.h"
#include "qwaylandinputdevice.h"
#include "qwaylanddecoration.h"
#include "qwaylandscreen.h"

#include <QtCore/QDebug>

QT_USE_NAMESPACE

QWaylandShellSurface::QWaylandShellSurface(struct wl_shell_surface *shell_surface, QWaylandWindow *window)
    : m_shell_surface(shell_surface)
    , m_window(window)
    , m_maximized(false)
    , m_fullscreen(false)
{
    wl_shell_surface_add_listener(m_shell_surface,&m_shell_surface_listener,this);
}

QWaylandShellSurface::~QWaylandShellSurface()
{
    wl_shell_surface_destroy(m_shell_surface);
}

void QWaylandShellSurface::resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges)
{
    wl_shell_surface_resize(m_shell_surface,inputDevice->wl_seat(),
                            inputDevice->serial(),
                            edges);
}

void QWaylandShellSurface::move(QWaylandInputDevice *inputDevice)
{
    wl_shell_surface_move(m_shell_surface,
                          inputDevice->wl_seat(),
                          inputDevice->serial());
}

void QWaylandShellSurface::setMaximized()
{
    m_maximized = true;
    m_size = m_window->window()->geometry().size();
    wl_shell_surface_set_maximized(m_shell_surface, 0);
}

void QWaylandShellSurface::setFullscreen()
{
    m_fullscreen = true;
    m_size = m_window->window()->geometry().size();
    wl_shell_surface_set_fullscreen(m_shell_surface, 0, 0, 0);
}

void QWaylandShellSurface::setNormal()
{
    if (m_fullscreen || m_maximized) {
        m_fullscreen = m_maximized = false;
        setTopLevel();
        QMargins m = m_window->frameMargins();
        m_window->configure(0, m_size.width() + m.left() + m.right(), m_size.height() + m.top() + m.bottom());
    }
}

void QWaylandShellSurface::setMinimized()
{
    // TODO: There's no wl_shell_surface API for this
}

void QWaylandShellSurface::setTopLevel()
{
    wl_shell_surface_set_toplevel(m_shell_surface);
}

void QWaylandShellSurface::updateTransientParent(QWindow *parent)
{
    QWaylandWindow *parent_wayland_window = static_cast<QWaylandWindow *>(parent->handle());
    if (!parent_wayland_window || !parent_wayland_window->shellSurface())
        return;

    // set_transient expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    QWindow *parentWin = m_window->window()->transientParent();
    transientPos -= parentWin->geometry().topLeft();
    if (parent_wayland_window->decoration()) {
        transientPos.setX(transientPos.x() + parent_wayland_window->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent_wayland_window->decoration()->margins().top());
    }

    uint32_t flags = 0;
    Qt::WindowFlags wf = m_window->window()->flags();
    if (wf.testFlag(Qt::ToolTip)
            || wf.testFlag(Qt::WindowTransparentForInput))
        flags |= WL_SHELL_SURFACE_TRANSIENT_INACTIVE;

    wl_shell_surface_set_transient(m_shell_surface,
                                   parent_wayland_window->wl_surface(),
                                   transientPos.x(),
                                   transientPos.y(),
                                   flags);
}

void QWaylandShellSurface::setClassName(const char *_class)
{
    wl_shell_surface_set_class(m_shell_surface, _class);
}

void QWaylandShellSurface::setTitle(const char *title)
{
    wl_shell_surface_set_title(m_shell_surface, title);
}

void QWaylandShellSurface::ping(void *data,
                                struct wl_shell_surface *wl_shell_surface,
                                uint32_t serial)
{
    Q_UNUSED(data);
    wl_shell_surface_pong(wl_shell_surface, serial);
}

void QWaylandShellSurface::configure(void *data,
                                     wl_shell_surface *wl_shell_surface,
                                     uint32_t edges,
                                     int32_t width,
                                     int32_t height)
{
    Q_UNUSED(wl_shell_surface);
    QWaylandShellSurface *shell_surface = static_cast<QWaylandShellSurface *>(data);
    shell_surface->m_window->configure(edges, width, height);
}

void QWaylandShellSurface::popup_done(void *data,
                                      struct wl_shell_surface *wl_shell_surface)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_shell_surface);
}

const wl_shell_surface_listener QWaylandShellSurface::m_shell_surface_listener = {
    QWaylandShellSurface::ping,
    QWaylandShellSurface::configure,
    QWaylandShellSurface::popup_done
};
