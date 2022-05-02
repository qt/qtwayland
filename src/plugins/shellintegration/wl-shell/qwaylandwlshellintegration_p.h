/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
**
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

class Q_WAYLAND_CLIENT_EXPORT QWaylandWlShellIntegration : public QWaylandShellIntegration
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
