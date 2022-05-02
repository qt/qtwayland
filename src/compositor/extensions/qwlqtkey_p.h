/****************************************************************************
**
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

#ifndef WLQTKEY_H
#define WLQTKEY_H

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

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/private/qwayland-server-qt-key-unstable-v1.h>

#include <wayland-util.h>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QKeyEvent;

namespace QtWayland {

class QtKeyExtensionGlobal : public QWaylandCompositorExtensionTemplate<QtKeyExtensionGlobal>, public QtWaylandServer::zqt_key_v1
{
    Q_OBJECT
public:
    QtKeyExtensionGlobal(QWaylandCompositor *compositor);

    bool postQtKeyEvent(QKeyEvent *event, QWaylandSurface *surface);

private:
    QWaylandCompositor *m_compositor = nullptr;
};

}

QT_END_NAMESPACE

#endif // WLQTKEY_H
