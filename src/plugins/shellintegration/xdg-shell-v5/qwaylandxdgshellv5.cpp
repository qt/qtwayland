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

#include "qwaylandxdgshellv5_p.h"
#include "qwaylandxdgpopupv5_p.h"
#include "qwaylandxdgsurfacev5_p.h"

#include <QtCore/QDebug>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgShellV5::QWaylandXdgShellV5(struct ::wl_registry *registry, uint32_t id)
    : QtWayland::xdg_shell_v5(registry, id, 1)
{
    use_unstable_version(QtWayland::xdg_shell_v5::version_current);
}

QWaylandXdgShellV5::~QWaylandXdgShellV5()
{
    xdg_shell_destroy(object());
}

QWaylandXdgSurfaceV5 *QWaylandXdgShellV5::createXdgSurface(QWaylandWindow *window)
{
    return new QWaylandXdgSurfaceV5(this, window);
}

QWaylandXdgPopupV5 *QWaylandXdgShellV5::createXdgPopup(QWaylandWindow *window, QWaylandInputDevice *inputDevice)
{
    QWaylandWindow *parentWindow = m_popups.empty() ? window->transientParent() : m_popups.last();
    if (!parentWindow)
        return nullptr;

    ::wl_surface *parentSurface = parentWindow->wlSurface();

    if (m_popupSerial == 0)
        m_popupSerial = inputDevice->serial();
    ::wl_seat *seat = inputDevice->wl_seat();

    QPoint position = window->geometry().topLeft() - parentWindow->geometry().topLeft();
    int x = position.x() + parentWindow->frameMargins().left();
    int y = position.y() + parentWindow->frameMargins().top();

    auto popup = new QWaylandXdgPopupV5(get_xdg_popup(window->wlSurface(), parentSurface, seat, m_popupSerial, x, y), window);
    m_popups.append(window);
    QObject::connect(popup, &QWaylandXdgPopupV5::destroyed, [this, window](){
        m_popups.removeOne(window);
        if (m_popups.empty())
            m_popupSerial = 0;
    });
    return popup;
}

void QWaylandXdgShellV5::xdg_shell_ping(uint32_t serial)
{
    pong(serial);
}

}

QT_END_NAMESPACE
