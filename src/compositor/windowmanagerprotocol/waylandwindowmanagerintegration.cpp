/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandwindowmanagerintegration.h"

#include "wayland_wrapper/wldisplay.h"
#include "wayland_wrapper/wlcompositor.h"

#include "wayland-server.h"
#include "wayland-windowmanager-server-protocol.h"

WindowManagerServerIntegration::WindowManagerServerIntegration(QObject *parent)
    : QObject(parent)
{
}

void WindowManagerServerIntegration::initialize(Wayland::Display *waylandDisplay)
{
    wl_display_add_global(waylandDisplay->handle(),&wl_windowmanager_interface,this,WindowManagerServerIntegration::bind_func);
}

void WindowManagerServerIntegration::removeClient(wl_client *client)
{
    WaylandManagedClient *managedClient = m_managedClients.take(client);
    delete managedClient;
}

WaylandManagedClient *WindowManagerServerIntegration::managedClient(wl_client *client) const
{
    return m_managedClients.value(client, 0);
}

void WindowManagerServerIntegration::setVisibilityOnScreen(wl_client *client, bool visible)
{
    struct wl_resource *win_mgr_resource  = resourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_CLIENT_ONSCREEN_VISIBILITY,visible);
}

void WindowManagerServerIntegration::setScreenOrientation(wl_client *client, wl_resource *output_resource, Qt::ScreenOrientation orientation)
{
    struct wl_resource *win_mgr_resource  = resourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_SET_SCREEN_ROTATION,output_resource, qint32(orientation));
}

// client -> server
void WindowManagerServerIntegration::updateWindowProperty(wl_client *client, wl_surface *surface, const char *name, struct wl_array *value)
{
    QVariant variantValue;
    QByteArray byteValue((const char*)value->data, value->size);
    QDataStream ds(&byteValue, QIODevice::ReadOnly);
    ds >> variantValue;

    emit windowPropertyChanged(client, surface, QString::fromLatin1(name), variantValue);
}

// server -> client
void WindowManagerServerIntegration::setWindowProperty(wl_client *client, wl_surface *surface, const QString &name, const QVariant &value)
{
    QByteArray byteValue;
    QDataStream ds(&byteValue, QIODevice::WriteOnly);
    ds << value;
    wl_array data;
    data.size = byteValue.size();
    data.data = (void*) byteValue.constData();
    data.alloc = 0;

    struct wl_resource *win_mgr_resource = resourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_SET_GENERIC_PROPERTY,surface, name.toLatin1().constData(),&data);
}


void WindowManagerServerIntegration::mapClientToProcess(wl_client *client, uint32_t processId)
{
    WaylandManagedClient *managedClient = m_managedClients.value(client, new WaylandManagedClient);
    managedClient->m_processId = processId;
    m_managedClients.insert(client, managedClient);
}

void WindowManagerServerIntegration::authenticateWithToken(wl_client *client, const char *token)
{
    Q_ASSERT(token != 0 && *token != 0);

    WaylandManagedClient *managedClient = m_managedClients.value(client, new WaylandManagedClient);
    managedClient->m_authenticationToken = QByteArray(token);
    m_managedClients.insert(client, managedClient);

    emit clientAuthenticated(client);
}

void WindowManagerServerIntegration::bind_func(struct wl_client *client, void *data,
                                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    WindowManagerServerIntegration *win_mgr = static_cast<WindowManagerServerIntegration *>(data);
    struct wl_resource *resource = wl_client_add_object(client,&wl_windowmanager_interface,&windowmanager_interface,id,data);
    win_mgr->registerResource(resource);
}


void WindowManagerServerIntegration::map_client_to_process(struct wl_client *client,
                                                           struct wl_resource *window_mgr_resource,
                                                           uint32_t process_id)
{
    WindowManagerServerIntegration *window_mgr = static_cast<WindowManagerServerIntegration *>(window_mgr_resource->data);
    window_mgr->mapClientToProcess(client,process_id);
}

void WindowManagerServerIntegration::authenticate_with_token(struct wl_client *client,
                                                             struct wl_resource *window_mgr_resource,
                                                             const char *wl_authentication_token)
{
    WindowManagerServerIntegration *window_mgr = static_cast<WindowManagerServerIntegration *>(window_mgr_resource->data);
    window_mgr->authenticateWithToken(client,wl_authentication_token);
}

void WindowManagerServerIntegration::update_generic_property(struct wl_client *client,
                                                             struct wl_resource *window_mgr_resource,
                                                             wl_resource *surface_resource,
                                                             const char *name,
                                                             struct wl_array *value)
{
    WindowManagerServerIntegration *window_mgr = static_cast<WindowManagerServerIntegration *>(window_mgr_resource->data);
    struct wl_surface *surface = static_cast<struct wl_surface *>(surface_resource->data);
    window_mgr->updateWindowProperty(client,surface,name,value);
}

const struct wl_windowmanager_interface WindowManagerServerIntegration::windowmanager_interface = {
    WindowManagerServerIntegration::map_client_to_process,
    WindowManagerServerIntegration::authenticate_with_token,
    WindowManagerServerIntegration::update_generic_property
};


/// ///
/// / WaylandManagedClient
/// ///

WaylandManagedClient::WaylandManagedClient()
    : m_processId(0)
{
}

qint64 WaylandManagedClient::processId() const
{
    return m_processId;
}

QByteArray WaylandManagedClient::authenticationToken() const
{
    return m_authenticationToken;
}
