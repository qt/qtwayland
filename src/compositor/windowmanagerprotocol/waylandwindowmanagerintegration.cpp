/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandwindowmanagerintegration.h"

#include "wayland_wrapper/qwldisplay_p.h"
#include "wayland_wrapper/qwlcompositor_p.h"

#include "compositor_api/qwaylandcompositor.h"

#include "wayland-server.h"
#include "wayland-windowmanager-server-protocol.h"

#include <QUrl>

QT_BEGIN_NAMESPACE

WindowManagerServerIntegration::WindowManagerServerIntegration(QWaylandCompositor *compositor, QObject *parent)
    : QObject(parent)
    , m_showIsFullScreen(false)
    , m_compositor(compositor)
{
}

WindowManagerServerIntegration::~WindowManagerServerIntegration()
{
}

void WindowManagerServerIntegration::initialize(QtWayland::Display *waylandDisplay)
{
    wl_display_add_global(waylandDisplay->handle(),&qt_windowmanager_interface,this,WindowManagerServerIntegration::bind_func);
}

void WindowManagerServerIntegration::setShowIsFullScreen(bool value)
{
    m_showIsFullScreen = value;
    struct wl_resource *resource;
    wl_list_for_each(resource,&client_resources, link) {
        qt_windowmanager_send_hints(resource, int32_t(m_showIsFullScreen));
    }
}

void WindowManagerServerIntegration::sendQuitMessage(wl_client *client)
{
    struct wl_resource *resource;
    wl_list_for_each(resource, &client_resources, link) {
        if (resource->client == client) {
            qt_windowmanager_send_quit(resource);
            return;
        }
    }
}

struct WindowManagerServerIntegrationClientData
{
    QByteArray url;
    WindowManagerServerIntegration *integration;
};

void WindowManagerServerIntegration::bind_func(struct wl_client *client, void *data,
                                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);

    WindowManagerServerIntegrationClientData *clientData = new WindowManagerServerIntegrationClientData;
    clientData->integration = static_cast<WindowManagerServerIntegration *>(data);

    wl_resource *resource = wl_client_add_object(client,&qt_windowmanager_interface,&windowmanager_interface,id,clientData);
    resource->destroy = WindowManagerServerIntegration::destroy_resource;
    clientData->integration->registerResource(resource);
    qt_windowmanager_send_hints(resource, int32_t(clientData->integration->m_showIsFullScreen));
}

void WindowManagerServerIntegration::destroy_resource(wl_resource *resource)
{
    WindowManagerServerIntegrationClientData *data = static_cast<WindowManagerServerIntegrationClientData *>(resource->data);

    delete data;
    free(resource);
}

void WindowManagerServerIntegration::open_url(struct wl_client *client,
                                              struct wl_resource *window_mgr_resource,
                                              uint32_t remaining,
                                              const char *url)
{
    WindowManagerServerIntegrationClientData *data = static_cast<WindowManagerServerIntegrationClientData *>(window_mgr_resource->data);
    WindowManagerServerIntegration *window_mgr = data->integration;

    data->url.append(url);

    if (!remaining) {
        window_mgr->m_compositor->openUrl(client, QUrl(QString::fromUtf8(data->url)));
        data->url = QByteArray();
    }
}

const struct qt_windowmanager_interface WindowManagerServerIntegration::windowmanager_interface = {
    WindowManagerServerIntegration::open_url
};

QT_END_NAMESPACE
