// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDIVISURFACE_H
#define QWAYLANDIVISURFACE_H

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include "qwayland-ivi-application.h"
#include "qwayland-ivi-controller.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;

class Q_WAYLANDCLIENT_EXPORT QWaylandIviSurface : public QtWayland::ivi_surface
        , public QWaylandShellSurface, public QtWayland::ivi_controller_surface
{
public:
    QWaylandIviSurface(struct ::ivi_surface *shell_surface, QWaylandWindow *window);
    QWaylandIviSurface(struct ::ivi_surface *shell_surface, QWaylandWindow *window,
                       struct ::ivi_controller_surface *iviControllerSurface);
    ~QWaylandIviSurface() override;

    void applyConfigure() override;

    std::any surfaceRole() const override { return ivi_surface::object(); };

private:
    void createExtendedSurface(QWaylandWindow *window);
    void ivi_surface_configure(int32_t width, int32_t height) override;
    void ivi_controller_surface_visibility(int32_t visibility) override;

    QWaylandWindow *m_window = nullptr;
    QSize m_pendingSize = {0, 0};
};

}

QT_END_NAMESPACE

#endif // QWAYLANDIVISURFACE_H
