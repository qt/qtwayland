// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandshellsurface_p.h"

#include <stdint.h>
#include <QtCore/QEvent>
#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/QtEvents>
#include <QtGui/QGuiApplication>

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindowManagerIntegrationPrivate {
public:
    QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay);
    bool m_blockPropertyUpdates = false;
    QWaylandDisplay *m_waylandDisplay = nullptr;
    QHash<QWindow*, QVariantMap> m_queuedProperties;
    bool m_showIsFullScreen = false;
};

QWaylandWindowManagerIntegrationPrivate::QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay)
    : m_waylandDisplay(waylandDisplay)
{

}

QWaylandWindowManagerIntegration::QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay)
    : d_ptr(new QWaylandWindowManagerIntegrationPrivate(waylandDisplay))
{
    waylandDisplay->addRegistryListener(&wlHandleListenerGlobal, this);
}

QWaylandWindowManagerIntegration::~QWaylandWindowManagerIntegration()
{

}

bool QWaylandWindowManagerIntegration::showIsFullScreen() const
{
    Q_D(const QWaylandWindowManagerIntegration);
    return d->m_showIsFullScreen;
}

void QWaylandWindowManagerIntegration::wlHandleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    Q_UNUSED(version);
    if (interface == QStringLiteral("qt_windowmanager"))
        static_cast<QWaylandWindowManagerIntegration *>(data)->init(registry, id, 1);
}

void QWaylandWindowManagerIntegration::windowmanager_hints(int32_t showIsFullScreen)
{
    Q_D(QWaylandWindowManagerIntegration);
    d->m_showIsFullScreen = showIsFullScreen;
}

void QWaylandWindowManagerIntegration::windowmanager_quit()
{
    QGuiApplication::quit();
}

void QWaylandWindowManagerIntegration::openUrl_helper(const QUrl &url)
{
    Q_ASSERT(isInitialized());
    QString data = url.toString();

    static const int chunkSize = 128;
    while (!data.isEmpty()) {
        QString chunk = data.left(chunkSize);
        data = data.mid(chunkSize);
        if (chunk.at(chunk.size() - 1).isHighSurrogate() && !data.isEmpty()) {
            chunk.append(data.at(0));
            data = data.mid(1);
        }
        open_url(!data.isEmpty(), chunk);
    }
}

bool QWaylandWindowManagerIntegration::openUrl(const QUrl &url)
{
    if (isInitialized()) {
        openUrl_helper(url);
        return true;
    }
    return QGenericUnixServices::openUrl(url);
}

bool QWaylandWindowManagerIntegration::openDocument(const QUrl &url)
{
    if (isInitialized()) {
        openUrl_helper(url);
        return true;
    }
    return QGenericUnixServices::openDocument(url);
}

QString QWaylandWindowManagerIntegration::portalWindowIdentifier(QWindow *window)
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
}

QT_END_NAMESPACE

#include "moc_qwaylandwindowmanagerintegration_p.cpp"
