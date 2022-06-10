// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDQTINTEGRATION_H
#define QWAYLANDQTINTEGRATION_H

#include <QtCore/qmutex.h>

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>
#include <QScopedPointer>
#include "qwayland-qt-shell-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;


class Q_WAYLANDCLIENT_EXPORT QWaylandQtShellIntegration
        : public QWaylandShellIntegrationTemplate<QWaylandQtShellIntegration>
        , public QtWayland::zqt_shell_v1
{
public:
    QWaylandQtShellIntegration();

    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;
};

}

QT_END_NAMESPACE

#endif //  QWAYLANDQTINTEGRATION_H
