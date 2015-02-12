/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: http://www.qt.io/licensing/
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
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

#include <private/qobject_p.h>

#include "wayland_wrapper/qwlcompositor_p.h"
#include "qwaylandcompositor.h"
#include "qwaylandclient.h"

#include <wayland-server.h>
#include <wayland-util.h>

QT_BEGIN_NAMESPACE

class QWaylandClientPrivate : public QObjectPrivate
{
public:
    QWaylandClientPrivate(wl_client *_client)
        : client(_client)
    {
        // Save client credentials
        wl_client_get_credentials(client, &pid, &uid, &gid);
    }

    ~QWaylandClientPrivate()
    {
    }

    static void client_destroy_callback(wl_listener *listener, void *data)
    {
        Q_UNUSED(data);

        QWaylandClient *client = reinterpret_cast<Listener *>(listener)->parent;
        Q_ASSERT(client != 0);
        QtWayland::Compositor::instance()->m_clients.removeOne(client);
        delete client;
    }

    wl_client *client;

    uid_t uid;
    gid_t gid;
    pid_t pid;

    struct Listener {
        wl_listener listener;
        QWaylandClient *parent;
    };
    Listener listener;
};

QWaylandClient::QWaylandClient(wl_client *client)
    : QObject(*new QWaylandClientPrivate(client))
{
    Q_D(QWaylandClient);

    // Destroy wrapper when the client goes away
    d->listener.parent = this;
    d->listener.listener.notify = QWaylandClientPrivate::client_destroy_callback;
    wl_client_add_destroy_listener(client, &d->listener.listener);
}

QWaylandClient::~QWaylandClient()
{
    Q_D(QWaylandClient);

    // Remove listener from signal
    wl_list_remove(&d->listener.listener.link);
}

QWaylandClient *QWaylandClient::fromWlClient(wl_client *wlClient)
{
    if (!wlClient)
        return 0;

    QWaylandClient *client = Q_NULLPTR;

    wl_listener *l = wl_client_get_destroy_listener(wlClient,
        QWaylandClientPrivate::client_destroy_callback);
    if (l)
        client = reinterpret_cast<QWaylandClientPrivate::Listener *>(
            wl_container_of(l, (QWaylandClientPrivate::Listener *)0, listener))->parent;

    if (!client) {
        // The original idea was to create QWaylandClient instances when
        // a client bound wl_compositor, but it's legal for a client to
        // bind several times resulting in multiple QWaylandClient
        // instances for the same wl_client therefore we create it from
        // here on demand
        client = new QWaylandClient(wlClient);
        QtWayland::Compositor::instance()->m_clients.append(client);
    }

    return client;
}

wl_client *QWaylandClient::client() const
{
    Q_D(const QWaylandClient);

    return d->client;
}

qint64 QWaylandClient::userId() const
{
    Q_D(const QWaylandClient);

    return d->uid;
}

qint64 QWaylandClient::groupId() const
{
    Q_D(const QWaylandClient);

    return d->gid;
}

qint64 QWaylandClient::processId() const
{
    Q_D(const QWaylandClient);

    return d->pid;
}

void QWaylandClient::kill(int sig)
{
    Q_D(QWaylandClient);

    ::kill(d->pid, sig);
}

void QWaylandClient::close()
{
    QtWayland::Compositor::instance()->waylandCompositor()->destroyClient(this);
}

QT_END_NAMESPACE
