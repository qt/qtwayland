// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLBUFFERMANAGER_H
#define QWLBUFFERMANAGER_H

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

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "qwlclientbuffer_p.h"
QT_BEGIN_NAMESPACE

class QWaylandCompositor;

namespace QtWayland {

class ClientBuffer;

class Q_WAYLANDCOMPOSITOR_EXPORT BufferManager : public QObject
{
public:
    BufferManager(QWaylandCompositor *compositor);
    ClientBuffer *getBuffer(struct ::wl_resource *buffer_resource);
    void registerBuffer(struct ::wl_resource *buffer_resource, ClientBuffer *clientBuffer);
private:
    friend struct buffer_manager_destroy_listener;
    static void destroy_listener_callback(wl_listener *listener, void *data);

    QHash<struct ::wl_resource *, ClientBuffer*> m_buffers;
    QWaylandCompositor *m_compositor = nullptr;
};

}
QT_END_NAMESPACE

#endif // QWLBUFFERMANAGER_H
