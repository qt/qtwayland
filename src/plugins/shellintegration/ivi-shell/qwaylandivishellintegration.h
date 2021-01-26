/****************************************************************************
**
** Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
** Contact: http://www.qt-project.org/legal
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
****************************************************************************/

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
class QWaylandIviControllerSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandIviShellIntegration : public QWaylandShellIntegration
{
public:
    QWaylandIviShellIntegration();

    bool initialize(QWaylandDisplay *display) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

private:
    uint32_t getNextUniqueSurfaceId();

private:
    QScopedPointer<QtWayland::ivi_application> m_iviApplication;
    QScopedPointer<QtWayland::ivi_controller> m_iviController;
    uint32_t m_lastSurfaceId = 0;
    uint32_t m_surfaceNumber = 0;
    bool m_useEnvSurfaceId = false;
    QRecursiveMutex m_mutex;
};

}

QT_END_NAMESPACE

#endif //  QWAYLANDIVIINTEGRATION_H
