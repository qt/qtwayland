// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDIVIINTEGRATION_H
#define QWAYLANDIVIINTEGRATION_H

#include <QtCore/qmutex.h>

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>
#include "qwayland-ivi-application.h"
#include "qwayland-ivi-controller.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;
class QWaylandIviController;

class Q_WAYLANDCLIENT_EXPORT QWaylandIviShellIntegration
    : public QWaylandShellIntegrationTemplate<QWaylandIviShellIntegration>,
      public QtWayland::ivi_application
{
public:
    QWaylandIviShellIntegration();

    bool initialize(QWaylandDisplay *display) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

private:
    uint32_t getNextUniqueSurfaceId();

private:
    QScopedPointer<QWaylandIviController> m_iviController;
    uint32_t m_lastSurfaceId = 0;
    uint32_t m_surfaceNumber = 0;
    bool m_useEnvSurfaceId = false;
    QRecursiveMutex m_mutex;
};

}

QT_END_NAMESPACE

#endif //  QWAYLANDIVIINTEGRATION_H
