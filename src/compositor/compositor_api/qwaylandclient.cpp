/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
