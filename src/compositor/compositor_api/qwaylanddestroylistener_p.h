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
