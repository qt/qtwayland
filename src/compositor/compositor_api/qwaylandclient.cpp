// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandclient.h"
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>


#include <wayland-server-core.h>
#include <wayland-util.h>

QT_BEGIN_NAMESPACE

class QWaylandClientPrivate : public QObjectPrivate
{
public:
    QWaylandClientPrivate(QWaylandCompositor *compositor, wl_client *_client)
        : compositor(compositor)
        , client(_client)
    {
        // Save client credentials
        wl_client_get_credentials(client, &pid, &uid, &gid);
    }

    ~QWaylandClientPrivate() override
    {
    }

    static void client_destroy_callback(wl_listener *listener, void *data)
    {
        Q_UNUSED(data);

        QWaylandClient *client = reinterpret_cast<Listener *>(listener)->parent;
        Q_ASSERT(client != nullptr);
        delete client;
    }

    QWaylandCompositor *compositor = nullptr;
    wl_client *client = nullptr;

    uid_t uid;
    gid_t gid;
    pid_t pid;

    struct Listener {
        wl_listener listener;
        QWaylandClient *parent = nullptr;
    };
    Listener listener;

    QWaylandClient::TextInputProtocols mTextInputProtocols = QWaylandClient::NoProtocol;
};

/*!
 * \qmltype WaylandClient
 * \nativetype QWaylandClient
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief Represents a client connecting to the WaylandCompositor.
 *
 * This type represents a client connecting to the compositor using the Wayland protocol.
 * It corresponds to the Wayland interface wl_client.
 */

/*!
 * \class QWaylandClient
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandClient class represents a client connecting to the QWaylandCompositor.
 *
 * This class corresponds to a client connecting to the compositor using the Wayland protocol.
 * It corresponds to the Wayland interface wl_client.
 */

/*!
 * Constructs a QWaylandClient for the \a compositor and the Wayland \a client.
 */
QWaylandClient::QWaylandClient(QWaylandCompositor *compositor, wl_client *client)
    : QObject(*new QWaylandClientPrivate(compositor, client))
{
    Q_D(QWaylandClient);

    // Destroy wrapper when the client goes away
    d->listener.parent = this;
    d->listener.listener.notify = QWaylandClientPrivate::client_destroy_callback;
    wl_client_add_destroy_listener(client, &d->listener.listener);

    QWaylandCompositorPrivate::get(compositor)->addClient(this);
}

/*!
 * Destroys the QWaylandClient.
 */
QWaylandClient::~QWaylandClient()
{
    Q_D(QWaylandClient);

    // Remove listener from signal
    wl_list_remove(&d->listener.listener.link);

    QWaylandCompositorPrivate::get(d->compositor)->removeClient(this);
}

/*!
 * Returns the QWaylandClient corresponding to the Wayland client \a wlClient and \a compositor.
 * If a QWaylandClient has not already been created for a client, it is
 * created and returned.
 */
QWaylandClient *QWaylandClient::fromWlClient(QWaylandCompositor *compositor, wl_client *wlClient)
{
    if (!wlClient)
        return nullptr;

    QWaylandClient *client = nullptr;

    wl_listener *l = wl_client_get_destroy_listener(wlClient,
        QWaylandClientPrivate::client_destroy_callback);
    if (l)
        client = reinterpret_cast<QWaylandClientPrivate::Listener *>(
            wl_container_of(l, (QWaylandClientPrivate::Listener *)nullptr, listener))->parent;

    if (!client) {
        // The original idea was to create QWaylandClient instances when
        // a client bound wl_compositor, but it's legal for a client to
        // bind several times resulting in multiple QWaylandClient
        // instances for the same wl_client therefore we create it from
        // here on demand
        client = new QWaylandClient(compositor, wlClient);
    }

    return client;
}

/*!
 * \qmlproperty WaylandCompositor QtWayland.Compositor::WaylandClient::compositor
 *
 * This property holds the compositor of this WaylandClient.
 */

/*!
 * \property QWaylandClient::compositor
 *
 * This property holds the compositor of this QWaylandClient.
 */
QWaylandCompositor *QWaylandClient::compositor() const
{
    Q_D(const QWaylandClient);

    return d->compositor;
}

/*!
 * Returns the Wayland client of this QWaylandClient.
 */
wl_client *QWaylandClient::client() const
{
    Q_D(const QWaylandClient);

    return d->client;
}

/*!
 * \qmlproperty int QtWayland.Compositor::WaylandClient::userId
 *
 * This property holds the user id of this WaylandClient.
 */

/*!
 * \property QWaylandClient::userId
 * \readonly
 *
 * This property holds the user id of this QWaylandClient.
 */
qint64 QWaylandClient::userId() const
{
    Q_D(const QWaylandClient);

    return d->uid;
}

/*!
 * \qmlproperty int QtWayland.Compositor::WaylandClient::groupId
 * \readonly
 *
 * This property holds the group id of this WaylandClient.
 */

/*!
 * \property QWaylandClient::groupId
 *
 * This property holds the group id of this QWaylandClient.
 */
qint64 QWaylandClient::groupId() const
{
    Q_D(const QWaylandClient);

    return d->gid;
}

/*!
 * \qmlproperty int QtWayland.Compositor::WaylandClient::processId
 * \readonly
 *
 * This property holds the process id of this WaylandClient.
 */

/*!
 * \property QWaylandClient::processId
 *
 * This property holds the process id of this QWaylandClient.
 */
qint64 QWaylandClient::processId() const
{
    Q_D(const QWaylandClient);

    return d->pid;
}

/*!
 * \qmlmethod void QtWayland.Compositor::WaylandClient::kill(signal)
 *
 * Kills the client with the specified \a signal.
 */

/*!
 * Kills the client with the specified \a signal.
 */
void QWaylandClient::kill(int signal)
{
    Q_D(QWaylandClient);

    ::kill(d->pid, signal);
}

/*!
 * \qmlmethod void QtWayland.Compositor::WaylandClient::close()
 *
 * Closes the client
 */

/*!
 * Closes the client.
 */
void QWaylandClient::close()
{
    Q_D(QWaylandClient);
    d->compositor->destroyClient(this);
}

QWaylandClient::TextInputProtocols QWaylandClient::textInputProtocols() const
{
    Q_D(const QWaylandClient);
    return d->mTextInputProtocols;
}

void QWaylandClient::setTextInputProtocols(TextInputProtocols p)
{
    Q_D(QWaylandClient);
    if (d->mTextInputProtocols != p)
        d->mTextInputProtocols = p;
}

QT_END_NAMESPACE

#include "moc_qwaylandclient.cpp"

