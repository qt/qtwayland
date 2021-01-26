/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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
****************************************************************************/

#include "qwaylanddestroylistener.h"
#include "qwaylanddestroylistener_p.h"

QT_BEGIN_NAMESPACE

QWaylandDestroyListenerPrivate::QWaylandDestroyListenerPrivate()
{
    listener.parent = this;
    listener.listener.notify = handler;
    wl_list_init(&listener.listener.link);
}

QWaylandDestroyListener::QWaylandDestroyListener(QObject *parent)
    : QObject(* new QWaylandDestroyListenerPrivate(), parent)
{
}
void QWaylandDestroyListener::listenForDestruction(::wl_resource *resource)
{
    Q_D(QWaylandDestroyListener);
    wl_resource_add_destroy_listener(resource, &d->listener.listener);
}

void QWaylandDestroyListener::reset()
{
    Q_D(QWaylandDestroyListener);
    wl_list_remove(&d->listener.listener.link);
    wl_list_init(&d->listener.listener.link);
}

void QWaylandDestroyListenerPrivate::handler(wl_listener *listener, void *data)
{
    QWaylandDestroyListenerPrivate *that = reinterpret_cast<Listener *>(listener)->parent;
    emit that->q_func()->fired(data);
}

QT_END_NAMESPACE
