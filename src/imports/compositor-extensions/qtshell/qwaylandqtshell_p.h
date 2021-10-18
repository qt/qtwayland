/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDQTSHELL_P_H
#define QWAYLANDQTSHELL_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/QWaylandSurfaceRole>

#include <QHash>

#include "qwayland-server-qt-shell-unstable-v1.h"
#include "qwaylandqtshell.h"

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

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQtShellPrivate
    : public QWaylandCompositorExtensionPrivate
    , public QtWaylandServer::zqt_shell_v1
{
    Q_DECLARE_PUBLIC(QWaylandQtShell)

public:
    QWaylandQtShellPrivate();
    static QWaylandQtShellPrivate *get(QWaylandQtShell *qtShell) { return qtShell->d_func(); }
    void unregisterQtShellSurface(QWaylandQtShellSurface *qtShellSurface);

    QList<QWaylandQtShellChrome *> m_chromes;

protected:
    void zqt_shell_v1_surface_create(Resource *resource, wl_resource *surface, uint32_t id) override;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQtShellSurfacePrivate
    : public QWaylandCompositorExtensionPrivate
    , public QtWaylandServer::zqt_shell_surface_v1
{
    Q_DECLARE_PUBLIC(QWaylandQtShellSurface)

public:
    QWaylandQtShellSurfacePrivate();
    static QWaylandQtShellSurfacePrivate *get(QWaylandQtShellSurface *qtShellSurface)
    {
        return qtShellSurface->d_func();
    }

    void updateFrameMargins();
    void configure(uint windowState, const QRect &newGeometry);

protected:
    void zqt_shell_surface_v1_destroy_resource(Resource *resource) override;
    void zqt_shell_surface_v1_destroy(Resource *resource) override;
    void zqt_shell_surface_v1_reposition(Resource *resource, int32_t x, int32_t y) override;
    void zqt_shell_surface_v1_set_size(Resource *resource, int32_t width, int32_t height) override;
    void zqt_shell_surface_v1_set_minimum_size(Resource *resource, int32_t width, int32_t height) override;
    void zqt_shell_surface_v1_set_maximum_size(Resource *resource, int32_t width, int32_t height) override;
    void zqt_shell_surface_v1_set_window_title(Resource *resource, const QString &title) override;
    void zqt_shell_surface_v1_set_window_flags(Resource *resource, uint32_t flags) override;
    void zqt_shell_surface_v1_change_window_state(Resource *resource, uint32_t state) override;
    void zqt_shell_surface_v1_ack_configure(Resource *resource, uint32_t serial) override;

    void zqt_shell_surface_v1_start_system_resize(Resource *resource, uint32_t serial, uint32_t edge) override;
    void zqt_shell_surface_v1_start_system_move(Resource *resource, uint32_t serial) override;

    void zqt_shell_surface_v1_raise(Resource *resource) override;
    void zqt_shell_surface_v1_lower(Resource *resource) override;

    void zqt_shell_surface_v1_request_activate(Resource *resource) override;

private:
    QWaylandQtShell *m_qtShell = nullptr;
    QWaylandSurface *m_surface = nullptr;
    QRect m_windowGeometry;
    QSize m_minimumSize;
    QSize m_maximumSize;
    uint m_windowFlags = 0;
    uint m_windowState = 0;
    QString m_windowTitle;
    QMargins m_frameMargins;
    bool m_positionSet = false;
    bool m_active = false;

    QPoint m_pendingPosition;
    bool m_pendingPositionValid = false;
    QSize m_pendingSize;

    uint32_t m_lastAckedConfigure = UINT32_MAX;
    QMap<uint32_t, QPair<uint, QRect> > m_pendingConfigures;

    QWaylandQtShellSurface::CapabilityFlags m_capabilities;

    static QWaylandSurfaceRole s_role;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTSHELL_P_H
