/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDXDGSURFACE_H
#define QWAYLANDXDGSURFACE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QSize>
#include <QtCore/QMargins>

#include <wayland-client.h>

#include <QtWaylandClient/private/qwayland-xdg-shell.h>
#include <QtWaylandClient/private/qwaylandclientexport_p.h>
#include "qwaylandshellsurface_p.h"

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandExtendedSurface;
class QWaylandXdgShell;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgSurface : public QWaylandShellSurface
        , public QtWayland::xdg_surface
{
    Q_OBJECT
public:
    QWaylandXdgSurface(QWaylandXdgShell *shell, QWaylandWindow *window);
    virtual ~QWaylandXdgSurface();

    using QtWayland::xdg_surface::resize;
    void resize(QWaylandInputDevice *inputDevice, enum resize_edge edges);

    void resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges) Q_DECL_OVERRIDE;

    using QtWayland::xdg_surface::move;
    void move(QWaylandInputDevice *inputDevice) Q_DECL_OVERRIDE;

    void setTitle(const QString &title) Q_DECL_OVERRIDE;
    void setAppId(const QString &appId) Q_DECL_OVERRIDE;

    void raise() Q_DECL_OVERRIDE;
    void lower() Q_DECL_OVERRIDE;
    void setContentOrientationMask(Qt::ScreenOrientations orientation) Q_DECL_OVERRIDE;
    void setWindowFlags(Qt::WindowFlags flags) Q_DECL_OVERRIDE;
    void sendProperty(const QString &name, const QVariant &value) Q_DECL_OVERRIDE;

    bool shellManagesActiveState() const Q_DECL_OVERRIDE { return true; }

    bool isFullscreen() const { return m_fullscreen; }
    bool isMaximized() const { return m_maximized; }

private:
    void setMaximized() Q_DECL_OVERRIDE;
    void setFullscreen() Q_DECL_OVERRIDE;
    void setNormal() Q_DECL_OVERRIDE;
    void setMinimized() Q_DECL_OVERRIDE;

    void setTopLevel() Q_DECL_OVERRIDE;
    void updateTransientParent(QWindow *parent) Q_DECL_OVERRIDE;

private:
    QWaylandWindow *m_window;
    QWaylandXdgShell* m_shell;
    bool m_maximized;
    bool m_minimized;
    bool m_fullscreen;
    bool m_active;
    QSize m_normalSize;
    QMargins m_margins;
    QWaylandExtendedSurface *m_extendedWindow;

    void xdg_surface_configure(int32_t width,
                               int32_t height,
                               struct wl_array *states,
                               uint32_t serial) Q_DECL_OVERRIDE;
    void xdg_surface_close() Q_DECL_OVERRIDE;

    friend class QWaylandWindow;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGSURFACE_H
