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

#include <QtCore/QDebug>

// the protocol files are generated with wayland-scanner, in the following manner:
// wayland-scanner client-header < windowmanager.xml > wayland-windowmanager-client-protocol.h
// wayland-scanner server-header < windowmanager.xml > wayland-windowmanager-server-protocol.h
// wayland-scanner code < windowmanager.xml > wayland-windowmanager-protocol.c
//
// wayland-scanner can be found from wayland sources.

class WindowManagerObject : public Wayland::Object<struct wl_object>
{
public:

    void mapClientToProcess(wl_client *client, uint32_t processId)
    {
        WindowManagerServerIntegration::instance()->mapClientToProcess(client, processId);
    }

};

void map_client_to_process(wl_client *client, struct wl_windowmanager *windowMgr, uint32_t processId)
{
    reinterpret_cast<WindowManagerObject *>(windowMgr)->mapClientToProcess(client, processId);
}

const static struct wl_windowmanager_interface windowmanager_interface = {
    map_client_to_process
};

WindowManagerServerIntegration *WindowManagerServerIntegration::m_instance = 0;

WindowManagerServerIntegration::WindowManagerServerIntegration(QObject *parent)
    : QObject(parent)
{
    m_instance = this;
}

void WindowManagerServerIntegration::initialize(Wayland::Display *waylandDisplay)
{
    m_windowManagerObject = new WindowManagerObject();
    waylandDisplay->addGlobalObject(m_windowManagerObject->base(),
                                    &wl_windowmanager_interface, &windowmanager_interface, 0);
}

void WindowManagerServerIntegration::removeClient(wl_client *client)
{
    m_clientToProcessId.remove(client);
}

void WindowManagerServerIntegration::mapClientToProcess(wl_client *client, uint32_t processId)
{
    m_clientToProcessId.insert(client, processId);
    emit clientMappedToProcess(client, processId);
}


qint64 WindowManagerServerIntegration::pidForClient(wl_client *client) const
{
    return m_clientToProcessId.value(client, 0);
}

WindowManagerServerIntegration *WindowManagerServerIntegration::instance()
{
    return m_instance;
}
