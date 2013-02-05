/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandwindowmanagerintegration.h"
#include "wayland-windowmanager-client-protocol.h"
#include "qwaylandscreen.h"
#include "qwaylandwindow.h"

#include <stdint.h>
#include <QtCore/QEvent>
#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/QtEvents>
#include <QtGui/QGuiApplication>

#include <QDebug>

QT_USE_NAMESPACE

class QWaylandWindowManagerIntegrationPrivate {
public:
    QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay);
    bool m_blockPropertyUpdates;
    QWaylandDisplay *m_waylandDisplay;
    struct wl_windowmanager *m_waylandWindowManager;
    QHash<QWindow*, QVariantMap> m_queuedProperties;
    bool m_showIsFullScreen;
};

QWaylandWindowManagerIntegrationPrivate::QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay)
    : m_blockPropertyUpdates(false)
    , m_waylandDisplay(waylandDisplay)
    , m_waylandWindowManager(0)
    , m_showIsFullScreen(false)
{

}

QWaylandWindowManagerIntegration *QWaylandWindowManagerIntegration::m_instance = 0;

QWaylandWindowManagerIntegration *QWaylandWindowManagerIntegration::createIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandWindowManagerIntegration(waylandDisplay);
}

QWaylandWindowManagerIntegration::QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay)
    : d_ptr(new QWaylandWindowManagerIntegrationPrivate(waylandDisplay))
{
    m_instance = this;
    waylandDisplay->addRegistryListener(&wlHandleListenerGlobal, this);
}

QWaylandWindowManagerIntegration::~QWaylandWindowManagerIntegration()
{

}

QWaylandWindowManagerIntegration *QWaylandWindowManagerIntegration::instance()
{
    return m_instance;
}

struct wl_windowmanager *QWaylandWindowManagerIntegration::windowManager() const
{
    Q_D(const QWaylandWindowManagerIntegration);
    return d->m_waylandWindowManager;
}

bool QWaylandWindowManagerIntegration::showIsFullScreen() const
{
    Q_D(const QWaylandWindowManagerIntegration);
    return d->m_showIsFullScreen;
}

void QWaylandWindowManagerIntegration::wlHandleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
    Q_UNUSED(version);
    if (strcmp(interface, "wl_windowmanager") == 0) {
        QWaylandWindowManagerIntegration *integration = static_cast<QWaylandWindowManagerIntegration *>(data);
        integration->d_ptr->m_waylandWindowManager =
                static_cast<struct wl_windowmanager *>(wl_registry_bind(registry, id, &wl_windowmanager_interface, 1));
        wl_windowmanager_add_listener(integration->d_ptr->m_waylandWindowManager, &windowmanager_listener, integration);
    }
}

const struct wl_windowmanager_listener QWaylandWindowManagerIntegration::windowmanager_listener = {
    QWaylandWindowManagerIntegration::handle_hints,
    QWaylandWindowManagerIntegration::handle_quit
};

void QWaylandWindowManagerIntegration::handle_hints(void *data, wl_windowmanager *ext, int32_t showIsFullScreen)
{
    Q_UNUSED(ext);
    QWaylandWindowManagerIntegration *self = static_cast<QWaylandWindowManagerIntegration *>(data);
    self->d_func()->m_showIsFullScreen = showIsFullScreen;
}

void QWaylandWindowManagerIntegration::handle_quit(void *data, wl_windowmanager *ext)
{
    Q_UNUSED(data);
    Q_UNUSED(ext);
    QGuiApplication::quit();
}

void QWaylandWindowManagerIntegration::mapClientToProcess(long long processId)
{
    Q_D(QWaylandWindowManagerIntegration);
    if (d->m_waylandWindowManager)
        wl_windowmanager_map_client_to_process(d->m_waylandWindowManager, (uint32_t) processId);
}

void QWaylandWindowManagerIntegration::authenticateWithToken(const QByteArray &token)
{
    Q_D(QWaylandWindowManagerIntegration);
    QByteArray authToken = token;
    if (authToken.isEmpty())
        authToken = qgetenv("WL_AUTHENTICATION_TOKEN");

    if (d->m_waylandWindowManager && !authToken.isEmpty()) {
        wl_windowmanager_authenticate_with_token(d->m_waylandWindowManager, authToken.constData());
    }
}

void QWaylandWindowManagerIntegration::openUrl_helper(const QUrl &url)
{
    Q_D(QWaylandWindowManagerIntegration);
    if (d->m_waylandWindowManager) {
        QByteArray data = url.toString().toUtf8();

        static const int chunkSize = 128;
        while (!data.isEmpty()) {
            QByteArray chunk = data.left(chunkSize);
            data = data.mid(chunkSize);
            wl_windowmanager_open_url(d->m_waylandWindowManager, !data.isEmpty(), chunk.constData());
        }
    }
}

bool QWaylandWindowManagerIntegration::openUrl(const QUrl &url)
{
    openUrl_helper(url);
    return true;
}

bool QWaylandWindowManagerIntegration::openDocument(const QUrl &url)
{
    openUrl_helper(url);
    return true;
}
