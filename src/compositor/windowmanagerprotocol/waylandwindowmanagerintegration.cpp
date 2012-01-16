/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
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

const struct wl_windowmanager_interface WindowManagerServerIntegration::windowmanager_interface = {
    WindowManagerServerIntegration::map_client_to_process,
    WindowManagerServerIntegration::authenticate_with_token,
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
