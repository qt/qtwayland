/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandwindowmanagerextension.h"

#include <wayland_wrapper/qwldisplay_p.h>
#include <wayland_wrapper/qwlcompositor_p.h>

#include <compositor_api/qwaylandclient.h>
#include <compositor_api/qwaylandcompositor.h>

#include <wayland-server.h>

#include <QUrl>

QT_BEGIN_NAMESPACE

WindowManagerServerIntegration::WindowManagerServerIntegration(QWaylandCompositor *compositor, QObject *parent)
    : QWaylandExtension(compositor)
    , QtWaylandServer::qt_windowmanager()
    , m_showIsFullScreen(false)
    , m_compositor(compositor)
{
}

WindowManagerServerIntegration::~WindowManagerServerIntegration()
{
}

void WindowManagerServerIntegration::initialize(QtWayland::Display *waylandDisplay)
{
    init(waylandDisplay->handle(), 1);
}

void WindowManagerServerIntegration::setShowIsFullScreen(bool value)
{
    m_showIsFullScreen = value;
    Q_FOREACH (Resource *resource, resourceMap().values()) {
        send_hints(resource->handle, static_cast<int32_t>(m_showIsFullScreen));
    }
}

void WindowManagerServerIntegration::sendQuitMessage(wl_client *client)
{
    Resource *resource = resourceMap().value(client);

    if (resource)
        send_quit(resource->handle);
}

void WindowManagerServerIntegration::windowmanager_bind_resource(Resource *resource)
{
    send_hints(resource->handle, static_cast<int32_t>(m_showIsFullScreen));
}

void WindowManagerServerIntegration::windowmanager_destroy_resource(Resource *resource)
{
    m_urls.remove(resource);
}

void WindowManagerServerIntegration::windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &newUrl)
{
    QString url = m_urls.value(resource, QString());

    url.append(newUrl);

    if (remaining)
        m_urls.insert(resource, url);
    else {
        m_urls.remove(resource);
        m_compositor->openUrl(QWaylandClient::fromWlClient(resource->client()), QUrl(url));
    }
}

QT_END_NAMESPACE
