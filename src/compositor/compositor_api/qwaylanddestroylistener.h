/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QWAYLANDDESTROYLISTENER_H
#define QWAYLANDDESTROYLISTENER_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandDestroyListenerPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandDestroyListener : public QObject
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
