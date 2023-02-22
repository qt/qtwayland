// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandxdgshell_p.h"
#include "qwaylandxdgshellintegration_p.h"
#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgShellIntegration::QWaylandXdgShellIntegration() : QWaylandShellIntegrationTemplate(4)
{
    connect(this, &QWaylandShellIntegrationTemplate::activeChanged, this, [this] {
        if (isActive()) {
            mXdgShell.reset(new QWaylandXdgShell(mDisplay, this));
        } else {
            mXdgShell.reset(nullptr);
            destroy();
        }
    });
}

QWaylandXdgShellIntegration::~QWaylandXdgShellIntegration()
{
    if (isActive())
        destroy();
}

bool QWaylandXdgShellIntegration::initialize(QWaylandDisplay *display)
{
    mDisplay = display;
    return QWaylandShellIntegrationTemplate::initialize(display);
}

void QWaylandXdgShellIntegration::xdg_wm_base_ping(uint32_t serial)
{
    pong(serial);
}

QWaylandShellSurface *QWaylandXdgShellIntegration::createShellSurface(QWaylandWindow *window)
{
    return new QWaylandXdgSurface(mXdgShell.get(), get_xdg_surface(window->wlSurface()), window);
}

void *QWaylandXdgShellIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    if (auto waylandWindow = static_cast<QWaylandWindow *>(window->handle())) {
        if (auto xdgSurface = qobject_cast<QWaylandXdgSurface *>(waylandWindow->shellSurface())) {
            return xdgSurface->nativeResource(resource);
        }
    }
    return nullptr;
}

}

QT_END_NAMESPACE
