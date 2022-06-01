/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandClient>

#include "qwaylandqtwindowmanager.h"
#include "qwaylandqtwindowmanager_p.h"

QT_BEGIN_NAMESPACE

QWaylandQtWindowManagerPrivate::QWaylandQtWindowManagerPrivate()
{
}

void QWaylandQtWindowManagerPrivate::windowmanager_bind_resource(Resource *resource)
{
    send_hints(resource->handle, static_cast<int32_t>(showIsFullScreen));
}

void QWaylandQtWindowManagerPrivate::windowmanager_destroy_resource(Resource *resource)
{
    urls.remove(resource);
}

void QWaylandQtWindowManagerPrivate::windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &newUrl)
{
    Q_Q(QWaylandQtWindowManager);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor from QWaylandQtWindowManager::windowmanager_open_url()";
        return;
    }

    QString url = urls.value(resource, QString());

    url.append(newUrl);

    if (remaining)
        urls.insert(resource, url);
    else {
        urls.remove(resource);
        q->openUrl(QWaylandClient::fromWlClient(compositor, resource->client()), QUrl(url));
    }
}

QWaylandQtWindowManager::QWaylandQtWindowManager()
    : QWaylandCompositorExtensionTemplate<QWaylandQtWindowManager>(*new QWaylandQtWindowManagerPrivate())
{
}

QWaylandQtWindowManager::QWaylandQtWindowManager(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandQtWindowManager>(compositor, *new QWaylandQtWindowManagerPrivate())
{
}

bool QWaylandQtWindowManager::showIsFullScreen() const
{
    Q_D(const QWaylandQtWindowManager);
    return d->showIsFullScreen;
}

void QWaylandQtWindowManager::setShowIsFullScreen(bool value)
{
    Q_D(QWaylandQtWindowManager);

    if (d->showIsFullScreen == value)
        return;

    d->showIsFullScreen = value;
    const auto resMap = d->resourceMap();
    for (QWaylandQtWindowManagerPrivate::Resource *resource : resMap) {
        d->send_hints(resource->handle, static_cast<int32_t>(d->showIsFullScreen));
    }
    Q_EMIT showIsFullScreenChanged();
}

void QWaylandQtWindowManager::sendQuitMessage(QWaylandClient *client)
{
    Q_D(QWaylandQtWindowManager);
    QWaylandQtWindowManagerPrivate::Resource *resource = d->resourceMap().value(client->client());

    if (resource)
        d->send_quit(resource->handle);
}

void QWaylandQtWindowManager::initialize()
{
    Q_D(QWaylandQtWindowManager);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandQtWindowManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *QWaylandQtWindowManager::interface()
{
    return QWaylandQtWindowManagerPrivate::interface();
}

QByteArray QWaylandQtWindowManager::interfaceName()
{
    return QWaylandQtWindowManagerPrivate::interfaceName();
}

QT_END_NAMESPACE

#include "moc_qwaylandqtwindowmanager.cpp"
