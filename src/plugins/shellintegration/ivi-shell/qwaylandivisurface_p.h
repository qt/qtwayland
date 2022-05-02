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
******************************************************************************/

#ifndef QWAYLANDIVISURFACE_H
#define QWAYLANDIVISURFACE_H

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include "qwayland-ivi-application.h"
#include "qwayland-ivi-controller.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;
class QWindow;
class QWaylandExtendedSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandIviSurface : public QtWayland::ivi_surface
        , public QWaylandShellSurface, public QtWayland::ivi_controller_surface
{
public:
    QWaylandIviSurface(struct ::ivi_surface *shell_surface, QWaylandWindow *window);
    QWaylandIviSurface(struct ::ivi_surface *shell_surface, QWaylandWindow *window,
                       struct ::ivi_controller_surface *iviControllerSurface);
    ~QWaylandIviSurface() override;

    void applyConfigure() override;

private:
    void createExtendedSurface(QWaylandWindow *window);
    void ivi_surface_configure(int32_t width, int32_t height) override;
    void ivi_controller_surface_visibility(int32_t visibility) override;

    QWaylandWindow *m_window = nullptr;
    QWaylandExtendedSurface *m_extendedWindow = nullptr;
    QSize m_pendingSize = {0, 0};
};

}

QT_END_NAMESPACE

#endif // QWAYLANDIVISURFACE_H
