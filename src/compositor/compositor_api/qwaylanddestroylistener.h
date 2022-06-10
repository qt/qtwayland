// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDDESTROYLISTENER_H
#define QWAYLANDDESTROYLISTENER_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandDestroyListenerPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandDestroyListener : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandDestroyListener)
public:
    QWaylandDestroyListener(QObject *parent = nullptr);
    void listenForDestruction(struct wl_resource *resource);
    void reset();

Q_SIGNALS:
    void fired(void *data);

};

QT_END_NAMESPACE

#endif  /*QWAYLANDDESTROYLISTENER_H*/
