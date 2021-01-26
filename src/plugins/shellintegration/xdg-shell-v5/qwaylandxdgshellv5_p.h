/****************************************************************************
**
** Copyright (C) 2016 Eurogiciel, author: <philippe.coval@eurogiciel.fr>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
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

#ifndef QWAYLANDXDGSHELLV5_H
#define QWAYLANDXDGSHELLV5_H

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

#include "qwayland-xdg-shell-unstable-v5_p.h"

#include <QtCore/QSize>
#include <QtCore/QVector>

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandXdgSurfaceV5;
class QWaylandXdgPopupV5;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgShellV5 : public QtWayland::xdg_shell_v5
{
public:
    QWaylandXdgShellV5(struct ::wl_registry *registry, uint32_t id);
    ~QWaylandXdgShellV5() override;

    QWaylandXdgSurfaceV5 *createXdgSurface(QWaylandWindow *window);
    QWaylandXdgPopupV5 *createXdgPopup(QWaylandWindow *window, QWaylandInputDevice *inputDevice);

private:
    void xdg_shell_ping(uint32_t serial) override;

    QVector<QWaylandWindow *> m_popups;
    uint m_popupSerial = 0;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGSHELLV5_H
