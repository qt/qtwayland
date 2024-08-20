// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/private/qwaylandxdgshell_p.h>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

/*!
    \qmltype XdgDecorationManagerV1
    \nativetype QWaylandXdgDecorationManagerV1
    \inqmlmodule QtWayland.Compositor.XdgShell
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations.

    The XdgDecorationManagerV1 extension provides a way for a compositor to announce support for
    server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    XdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    To provide the functionality of the extension in a compositor, create an instance of the
    XdgDecorationManagerV1 component and add it to the list of extensions supported by the compositor:

    \qml
    import QtWayland.Compositor

    WaylandCompositor {
        // Xdg decoration manager assumes xdg-shell is being used
        XdgShell {
            onToplevelCreated: // ...
        }
        XdgDecorationManagerV1 {
            // Provide a hint to clients that support the extension they should use server-side
            // decorations.
            preferredMode: XdgToplevel.ServerSideDecoration
        }
    }
    \endqml

    \sa QWaylandXdgToplevel::decorationMode, {Server Side Decoration Compositor}
*/

/*!
    \class QWaylandXdgDecorationManagerV1
    \inmodule QtWaylandCompositor
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations.

    The QWaylandXdgDecorationManagerV1 extension provides a way for a compositor to announce support
    for server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    QWaylandXdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    \sa QWaylandXdgToplevel::decorationMode
*/

/*!
    Constructs a QWaylandXdgDecorationManagerV1 object.
*/
QWaylandXdgDecorationManagerV1::QWaylandXdgDecorationManagerV1()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>(*new QWaylandXdgDecorationManagerV1Private)
{
}

/*!
    Initializes the extension.
*/
void QWaylandXdgDecorationManagerV1::initialize()
{
    Q_D(QWaylandXdgDecorationManagerV1);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandXdgDecorationV1";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
    \qmlproperty string XdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
/*!
    \property QWaylandXdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
QWaylandXdgToplevel::DecorationMode QWaylandXdgDecorationManagerV1::preferredMode() const
{
    Q_D(const QWaylandXdgDecorationManagerV1);
    return d->m_preferredMode;
}

void QWaylandXdgDecorationManagerV1::setPreferredMode(QWaylandXdgToplevel::DecorationMode preferredMode)
{
    Q_D(QWaylandXdgDecorationManagerV1);
    if (d->m_preferredMode == preferredMode)
        return;

    d->m_preferredMode = preferredMode;
    emit preferredModeChanged();
}

/*!
    Returns the Wayland interface for the QWaylandXdgDecorationManagerV1.
*/
const wl_interface *QWaylandXdgDecorationManagerV1::interface()
{
    return QWaylandXdgDecorationManagerV1Private::interface();
}

void QWaylandXdgDecorationManagerV1Private::zxdg_decoration_manager_v1_get_toplevel_decoration(
        Resource *resource, uint id, wl_resource *toplevelResource)
{
    Q_Q(QWaylandXdgDecorationManagerV1);

    auto *toplevel = QWaylandXdgToplevel::fromResource(toplevelResource);
    if (!toplevel) {
        qWarning() << "Couldn't find toplevel for decoration";
        return;
    }

    //TODO: verify that the xdg surface is unconfigured, and post protocol error/warning

    auto *toplevelPrivate = QWaylandXdgToplevelPrivate::get(toplevel);

    if (toplevelPrivate->m_decoration) {
        qWarning() << "zxdg_decoration_manager_v1.get_toplevel_decoration:"
                   << toplevel << "already has a decoration object, ignoring";
        //TODO: protocol error as well?
        return;
    }

    new QWaylandXdgToplevelDecorationV1(toplevel, q, resource->client(), id);
}

QWaylandXdgToplevelDecorationV1::QWaylandXdgToplevelDecorationV1(QWaylandXdgToplevel *toplevel,
                                                                 QWaylandXdgDecorationManagerV1 *manager,
                                                                 wl_client *client, int id)
    : QtWaylandServer::zxdg_toplevel_decoration_v1(client, id, /*version*/ 1)
    , m_toplevel(toplevel)
    , m_manager(manager)
{
    Q_ASSERT(toplevel);
    auto *toplevelPrivate = QWaylandXdgToplevelPrivate::get(toplevel);
    Q_ASSERT(!toplevelPrivate->m_decoration);
    toplevelPrivate->m_decoration = this;
    sendConfigure(manager->preferredMode());
}

QWaylandXdgToplevelDecorationV1::~QWaylandXdgToplevelDecorationV1()
{
    QWaylandXdgToplevelPrivate::get(m_toplevel)->m_decoration = nullptr;
}

void QWaylandXdgToplevelDecorationV1::sendConfigure(QWaylandXdgToplevelDecorationV1::DecorationMode mode)
{
    if (configuredMode() == mode)
        return;

    switch (mode) {
    case DecorationMode::ClientSideDecoration:
        send_configure(mode_client_side);
        break;
    case DecorationMode::ServerSideDecoration:
        send_configure(mode_server_side);
        break;
    default:
        qWarning() << "Illegal mode in QWaylandXdgToplevelDecorationV1::sendConfigure" << mode;
        break;
    }

    m_configuredMode = mode;
    emit m_toplevel->decorationModeChanged();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = mode;
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_unset_mode(Resource *resource)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = 0;
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::handleClientPreferredModeChanged()
{
    if (m_clientPreferredMode != m_configuredMode) {
        if (m_clientPreferredMode == 0)
            sendConfigure(m_manager->preferredMode());
        else
            sendConfigure(DecorationMode(m_clientPreferredMode));
    }
}

QT_END_NAMESPACE

#include "moc_qwaylandxdgdecorationv1.cpp"
