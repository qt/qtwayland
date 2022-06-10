// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

#include "moc_qwaylanddestroylistener.cpp"
