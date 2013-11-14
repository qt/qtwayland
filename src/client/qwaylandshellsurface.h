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

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

#include <QtCore/QSize>

#include <wayland-client.h>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qwaylandclientexport.h>

QT_BEGIN_NAMESPACE

class QWaylandWindow;
class QWaylandInputDevice;
class QWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellSurface : public QtWayland::wl_shell_surface
{
public:
    QWaylandShellSurface(struct ::wl_shell_surface *shell_surface, QWaylandWindow *window);
    ~QWaylandShellSurface();

    using QtWayland::wl_shell_surface::resize;
    void resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges);

    using QtWayland::wl_shell_surface::move;
    void move(QWaylandInputDevice *inputDevice);

private:
    void setMaximized();
    void setFullscreen();
    void setNormal();
    void setMinimized();

    void setTopLevel();
    void updateTransientParent(QWindow *parent);
    void setPopup(QWaylandWindow *parent, QWaylandInputDevice *device, int serial);

    QWaylandWindow *m_window;
    bool m_maximized;
    bool m_fullscreen;
    QSize m_size;

    void shell_surface_ping(uint32_t serial) Q_DECL_OVERRIDE;
    void shell_surface_configure(uint32_t edges,
                                 int32_t width,
                                 int32_t height) Q_DECL_OVERRIDE;
    void shell_surface_popup_done() Q_DECL_OVERRIDE;

    friend class QWaylandWindow;
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_H
