/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#include "qwaylandxdgshellv5integration_p.h"
#include "qwaylandxdgsurfacev5_p.h"
#include "qwaylandxdgpopupv5_p.h"
#include "qwaylandxdgshellv5_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellV5Integration::initialize(QWaylandDisplay *display)
{
    const auto globals = display->globals();
    for (QWaylandDisplay::RegistryGlobal global : globals) {
        if (global.interface == QLatin1String("xdg_shell")) {
            m_xdgShell.reset(new QWaylandXdgShellV5(display->wl_registry(), global.id));
            break;
        }
    }

    if (!m_xdgShell) {
        qWarning() << "Couldn't find global xdg_shell for xdg-shell unstable v5";
        return false;
    }

    qCWarning(lcQpaWayland) << "\"xdg-shell-v5\" is a deprecated shell extension, prefer using"
                            << "\"xdg-shell-v6\" or \"xdg-shell\" if supported by the compositor"
                            << "by setting the environment variable QT_WAYLAND_SHELL_INTEGRATION";

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandXdgShellV5Integration::createShellSurface(QWaylandWindow *window)
{
    QWaylandInputDevice *inputDevice = window->display()->lastInputDevice();
    if (window->window()->type() == Qt::WindowType::Popup && inputDevice) {
        if (auto *popup = m_xdgShell->createXdgPopup(window, inputDevice))
            return popup;

        qWarning(lcQpaWayland) << "Failed to create xdg-popup v5 for window" << window->window()
                               << "falling back to creating an xdg-surface";
    }

    return m_xdgShell->createXdgSurface(window);
}

void QWaylandXdgShellV5Integration::handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus) {
    if (newFocus && qobject_cast<QWaylandXdgPopupV5 *>(newFocus->shellSurface()))
        m_display->handleWindowActivated(newFocus);
    if (oldFocus && qobject_cast<QWaylandXdgPopupV5 *>(oldFocus->shellSurface()))
        m_display->handleWindowDeactivated(oldFocus);
}

}

QT_END_NAMESPACE
