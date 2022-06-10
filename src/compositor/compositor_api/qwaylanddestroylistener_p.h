// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTWAYLAND_QWLLISTENER_H
#define QTWAYLAND_QWLLISTENER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/QWaylandDestroyListener>

#include <QtCore/private/qobject_p.h>

#include <wayland-server-core.h>

QT_BEGIN_NAMESPACE

class QWaylandDestroyListenerPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QWaylandDestroyListener)

    QWaylandDestroyListenerPrivate();

    static void handler(wl_listener *listener, void *data);

    struct Listener {
        wl_listener listener;
        QWaylandDestroyListenerPrivate *parent = nullptr;
    };
    Listener listener;
};

QT_END_NAMESPACE

#endif
