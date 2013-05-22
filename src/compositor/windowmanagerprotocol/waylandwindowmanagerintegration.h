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

#ifndef WAYLANDWINDOWMANAGERINTEGRATION_H
#define WAYLANDWINDOWMANAGERINTEGRATION_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandresourcecollection.h>

#include <qwindowdefs.h>
#include <stdint.h>

#include <QObject>
#include <QMap>
#include <QVariant>

struct wl_client;
struct wl_object;

QT_BEGIN_NAMESPACE

namespace QtWayland {
    class Display;
}

class WindowManagerObject;
class WaylandManagedClient;
class QWaylandCompositor;

class Q_COMPOSITOR_EXPORT WindowManagerServerIntegration : public QObject, private QtWayland::ResourceCollection
{
    Q_OBJECT
public:
    WindowManagerServerIntegration(QWaylandCompositor *compositor, QObject *parent = 0);
    ~WindowManagerServerIntegration();

    void initialize(QtWayland::Display *waylandDisplay);

    void setShowIsFullScreen(bool value);
    void sendQuitMessage(wl_client *client);

private:
    bool m_showIsFullScreen;
    QWaylandCompositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                                          uint32_t version, uint32_t id);

    static void destroy_resource(wl_resource *resource);
    static void map_client_to_process(struct wl_client *client,
                                      struct wl_resource *windowMgrResource,
                                      uint32_t processId);
    static void authenticate_with_token(struct wl_client *client,
                                        struct wl_resource *windowMgrResource,
                                        const char *wl_authentication_token);
    static void open_url(struct wl_client *client,
                         struct wl_resource *window_mgr_resource,
                         uint32_t remaining,
                         const char *url);
    static const struct qt_windowmanager_interface windowmanager_interface;
};

QT_END_NAMESPACE

#endif // WAYLANDWINDOWMANAGERINTEGRATION_H
