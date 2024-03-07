// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandplatformservices_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandshellsurface_p.h"
#include "qwaylandwindowmanagerintegration_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandPlatformServices::QWaylandPlatformServices(QWaylandDisplay *display)
    : m_display(display) { }

bool QWaylandPlatformServices::openUrl(const QUrl &url)
{
    if (auto windowManagerIntegration = m_display->windowManagerIntegration()) {
        windowManagerIntegration->openUrl(url);
        return true;
    }
    return QGenericUnixServices::openUrl(url);
}

bool QWaylandPlatformServices::openDocument(const QUrl &url)
{
    if (auto windowManagerIntegration = m_display->windowManagerIntegration()) {
        windowManagerIntegration->openUrl(url);
        return true;
    }
    return QGenericUnixServices::openDocument(url);
}

QString QWaylandPlatformServices::portalWindowIdentifier(QWindow *window)
{
    if (window && window->handle()) {
        auto shellSurface = static_cast<QWaylandWindow *>(window->handle())->shellSurface();
        if (shellSurface) {
            const QString handle = shellSurface->externWindowHandle();
            return QLatin1String("wayland:") + handle;
        }
    }
    return QString();
}
} // namespace QtWaylandClient

QT_END_NAMESPACE

#include "moc_qwaylandplatformservices_p.cpp"
