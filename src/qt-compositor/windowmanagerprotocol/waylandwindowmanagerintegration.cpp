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

#include "waylandobject.h"
#include "wayland_wrapper/wldisplay.h"
#include "wayland_wrapper/wlcompositor.h"

#include "wayland-server.h"
#include "wayland-windowmanager-server-protocol.h"

// the protocol files are generated with wayland-scanner, in the following manner:
// wayland-scanner client-header < windowmanager.xml > wayland-windowmanager-client-protocol.h
// wayland-scanner server-header < windowmanager.xml > wayland-windowmanager-server-protocol.h
// wayland-scanner code < windowmanager.xml > wayland-windowmanager-protocol.c
//
// wayland-scanner can be found from wayland sources.

class WindowManagerObject
{
public:

    void mapClientToProcess(wl_client *client, uint32_t processId)
    {
        WindowManagerServerIntegration::instance()->mapClientToProcess(client, processId);
    }

    void authenticateWithToken(wl_client *client, const char *authenticationToken)
    {
        WindowManagerServerIntegration::instance()->authenticateWithToken(client, authenticationToken);
    }

    void changeScreenVisibility(wl_client *client, int visible)
    {
        WindowManagerServerIntegration::instance()->setVisibilityOnScreen(client, visible);
    }

    void updateWindowProperty(wl_client *client, wl_surface *surface, const char *name, struct wl_array *value)
    {
        WindowManagerServerIntegration::instance()->updateWindowProperty(client, surface, name, value);
    }

    static void bind_func(struct wl_client *client, void *data,
                                          uint32_t version, uint32_t id);
    struct wl_resource *getWindowManagerResourceForClient(struct wl_client *client) {
        for (int i = 0; i < m_client_resources.size(); i++) {
            if (m_client_resources.at(i)->client == client) {
                return m_client_resources.at(i);
            }
        }
        return 0;
    }

private:
    QList<struct wl_resource *>m_client_resources;

};

void map_client_to_process(wl_client *client, struct wl_resource *windowMgrResource, uint32_t processId)
{
    WindowManagerObject *windowMgr = static_cast<WindowManagerObject *>(windowMgrResource->data);
    windowMgr->mapClientToProcess(client,processId);
}

void authenticate_with_token(wl_client *client, struct wl_resource *windowMgrResource, const char *wl_authentication_token)
{
    WindowManagerObject *windowMgr = static_cast<WindowManagerObject *>(windowMgrResource->data);
    windowMgr->authenticateWithToken(client,wl_authentication_token);
}

void update_generic_property(wl_client *client, struct wl_resource *windowMgrResource, wl_resource *surfaceResource, const char *name, struct wl_array *value)
{
    WindowManagerObject *windowMgr = static_cast<WindowManagerObject *>(windowMgrResource->data);
    struct wl_surface *surface = static_cast<struct wl_surface *>(surfaceResource->data);
    windowMgr->updateWindowProperty(client,surface,name,value);
}

const static struct wl_windowmanager_interface windowmanager_interface = {
    map_client_to_process,
    authenticate_with_token,
    update_generic_property
};

void WindowManagerObject::bind_func(struct wl_client *client, void *data,
                                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    WindowManagerObject *win_mgr_object= static_cast<WindowManagerObject *>(data);
    struct wl_resource *resource =wl_client_add_object(client,&wl_windowmanager_interface,&windowmanager_interface,id,data);
    for (int i = 0; i < win_mgr_object->m_client_resources.size(); i++) {
        struct wl_client *existing_client = win_mgr_object->m_client_resources.at(i)->client;
        Q_ASSERT(client != existing_client);
    }
    win_mgr_object->m_client_resources.append(resource);
}

WindowManagerServerIntegration *WindowManagerServerIntegration::m_instance = 0;

WindowManagerServerIntegration::WindowManagerServerIntegration(QObject *parent)
    : QObject(parent)
{
    m_instance = this;
}

void WindowManagerServerIntegration::initialize(Wayland::Display *waylandDisplay)
{
    m_windowManagerObject = new WindowManagerObject();
    wl_display_add_global(waylandDisplay->handle(),&wl_windowmanager_interface,m_windowManagerObject,WindowManagerObject::bind_func);
}

void WindowManagerServerIntegration::removeClient(wl_client *client)
{
    WaylandManagedClient *managedClient = m_managedClients.take(client);
    delete managedClient;
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

void WindowManagerServerIntegration::setVisibilityOnScreen(wl_client *client, bool visible)
{
    struct wl_resource *win_mgr_resource  = m_windowManagerObject->getWindowManagerResourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_CLIENT_ONSCREEN_VISIBILITY,visible);
}

void WindowManagerServerIntegration::setScreenOrientation(wl_client *client, wl_object *output, Qt::ScreenOrientation orientation)
{
    struct wl_resource *win_mgr_resource  = m_windowManagerObject->getWindowManagerResourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_SET_SCREEN_ROTATION,output, qint32(orientation));
}

// client -> server
void WindowManagerServerIntegration::updateWindowProperty(wl_client *client, wl_surface *surface, const char *name, struct wl_array *value)
{
    QVariant variantValue;
    QByteArray byteValue((const char*)value->data, value->size);
    QDataStream ds(&byteValue, QIODevice::ReadOnly);
    ds >> variantValue;

    emit windowPropertyChanged(client, surface, QString(name), variantValue);
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

    struct wl_resource *win_mgr_resource = m_windowManagerObject->getWindowManagerResourceForClient(client);
    wl_resource_post_event(win_mgr_resource,WL_WINDOWMANAGER_SET_GENERIC_PROPERTY,surface, name.toLatin1().constData(),&data);
}

WaylandManagedClient *WindowManagerServerIntegration::managedClient(wl_client *client) const
{
    return m_managedClients.value(client, 0);
}

WindowManagerServerIntegration *WindowManagerServerIntegration::instance()
{
    return m_instance;
}

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
