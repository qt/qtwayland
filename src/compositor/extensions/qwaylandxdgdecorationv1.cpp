/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/private/qwaylandxdgshell_p.h>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

QWaylandXdgDecorationManagerV1::QWaylandXdgDecorationManagerV1()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>(*new QWaylandXdgDecorationManagerV1Private)
{
}

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

QWaylandXdgToplevel::DecorationMode QWaylandXdgDecorationManagerV1::preferredMode() const
{
    Q_D(const QWaylandXdgDecorationManagerV1);
    return d->m_defaultMode;
}

void QWaylandXdgDecorationManagerV1::setPreferredMode(QWaylandXdgToplevel::DecorationMode defaultMode)
{
    Q_D(QWaylandXdgDecorationManagerV1);
    if (d->m_defaultMode == defaultMode)
        return;

    d->m_defaultMode = defaultMode;
    emit defaultModeChanged();
}

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
    toplevelPrivate->m_decoration.reset(this);
    sendConfigure(manager->preferredMode());
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
    auto *toplevelPrivate = QWaylandXdgToplevelPrivate::get(m_toplevel);
    toplevelPrivate->m_decoration.reset();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode)
{
    m_clientPreferredMode = DecorationMode(mode);
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_unset_mode(Resource *resource)
{
    m_clientPreferredMode = DecorationMode::DefaultDecorationMode;
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::handleClientPreferredModeChanged()
{
    if (m_clientPreferredMode != m_configuredMode) {
        if (m_clientPreferredMode == DecorationMode::DefaultDecorationMode)
            sendConfigure(m_manager->preferredMode());
        else
            sendConfigure(m_clientPreferredMode);
    }
}

QT_END_NAMESPACE
