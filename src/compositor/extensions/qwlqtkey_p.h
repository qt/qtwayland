// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
#include <QtCore/private/qglobal_p.h>

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
