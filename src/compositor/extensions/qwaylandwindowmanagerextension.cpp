/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qwaylandwindowmanagerextension.h"
#include "qwaylandwindowmanagerextension_p.h"

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandClient>

#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

QWaylandWindowManagerExtension::QWaylandWindowManagerExtension(QWaylandCompositor *compositor, QObject *parent)
    : QWaylandExtensionTemplate(*new QWaylandWindowManagerExtensionPrivate(compositor), parent)
{
}

QWaylandWindowManagerExtensionPrivate::QWaylandWindowManagerExtensionPrivate(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplatePrivate(compositor)
    , m_showIsFullScreen(false)
    , m_compositor(compositor)
{
    init(compositor->waylandDisplay(), 1);
}

void QWaylandWindowManagerExtension::setShowIsFullScreen(bool value)
{
    Q_D(QWaylandWindowManagerExtension);
    d->m_showIsFullScreen = value;
    Q_FOREACH (QWaylandWindowManagerExtensionPrivate::Resource *resource, d->resourceMap().values()) {
        d->send_hints(resource->handle, static_cast<int32_t>(d->m_showIsFullScreen));
    }
}

void QWaylandWindowManagerExtension::sendQuitMessage(wl_client *client)
{
    Q_D(QWaylandWindowManagerExtension);
    QWaylandWindowManagerExtensionPrivate::Resource *resource = d->resourceMap().value(client);

    if (resource)
        d->send_quit(resource->handle);
}

void QWaylandWindowManagerExtensionPrivate::windowmanager_bind_resource(Resource *resource)
{
    send_hints(resource->handle, static_cast<int32_t>(m_showIsFullScreen));
}

void QWaylandWindowManagerExtensionPrivate::windowmanager_destroy_resource(Resource *resource)
{
    m_urls.remove(resource);
}

void QWaylandWindowManagerExtensionPrivate::windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &newUrl)
{
    Q_Q(QWaylandWindowManagerExtension);
    QString url = m_urls.value(resource, QString());

    url.append(newUrl);

    if (remaining)
        m_urls.insert(resource, url);
    else {
        m_urls.remove(resource);
        q->openUrl(QWaylandClient::fromWlClient(m_compositor, resource->client()), QUrl(url));
    }
}

const struct wl_interface *QWaylandWindowManagerExtension::interface()
{
    return QWaylandWindowManagerExtensionPrivate::interface();
}

QByteArray QWaylandWindowManagerExtension::interfaceName()
{
    return QWaylandWindowManagerExtensionPrivate::interfaceName();
}


QT_END_NAMESPACE
