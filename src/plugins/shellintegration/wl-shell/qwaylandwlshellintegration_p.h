// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDWLSHELLINTEGRATION_P_H
#define QWAYLANDWLSHELLINTEGRATION_P_H

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

#include <private/qwayland-wayland.h>

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLANDCLIENT_EXPORT QWaylandWlShellIntegration : public QWaylandShellIntegration
{
public:
    QWaylandWlShellIntegration() {}
    bool initialize(QWaylandDisplay *) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;
    void *nativeResourceForWindow(const QByteArray &resource, QWindow *window) override;

private:
    QtWayland::wl_shell *m_wlShell = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDWLSHELLINTEGRATION_P_H
