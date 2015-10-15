/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylanddisplay_p.h"

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
    bool m_blockPropertyUpdates;
    QWaylandDisplay *m_waylandDisplay;
    QHash<QWindow*, QVariantMap> m_queuedProperties;
    bool m_showIsFullScreen;
};

QWaylandWindowManagerIntegrationPrivate::QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay)
    : m_blockPropertyUpdates(false)
    , m_waylandDisplay(waylandDisplay)
    , m_showIsFullScreen(false)
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

QByteArray QWaylandWindowManagerIntegration::desktopEnvironment() const
{
    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!xdgCurrentDesktop.isEmpty())
        return xdgCurrentDesktop.toUpper(); // KDE, GNOME, UNITY, LXDE, MATE, XFCE...

    // Classic fallbacks
    if (!qEnvironmentVariableIsEmpty("KDE_FULL_SESSION"))
        return QByteArrayLiteral("KDE");
    if (!qEnvironmentVariableIsEmpty("GNOME_DESKTOP_SESSION_ID"))
        return QByteArrayLiteral("GNOME");

    // Fallback to checking $DESKTOP_SESSION (unreliable)
    const QByteArray desktopSession = qgetenv("DESKTOP_SESSION");
    if (desktopSession == "gnome")
        return QByteArrayLiteral("GNOME");
    if (desktopSession == "xfce")
        return QByteArrayLiteral("XFCE");

    return QByteArrayLiteral("UNKNOWN");
}

void QWaylandWindowManagerIntegration::openUrl_helper(const QUrl &url)
{
    Q_ASSERT(isInitialized());
    QByteArray data = url.toString().toUtf8();

    static const int chunkSize = 128;
    while (!data.isEmpty()) {
        QByteArray chunk = data.left(chunkSize);
        data = data.mid(chunkSize);
        open_url(!data.isEmpty(), QString::fromUtf8(chunk));
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

}

QT_END_NAMESPACE
