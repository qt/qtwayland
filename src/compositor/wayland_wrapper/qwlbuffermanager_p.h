/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
#include "qwlclientbuffer_p.h"
QT_BEGIN_NAMESPACE

class QWaylandCompositor;

namespace QtWayland {

class ClientBuffer;

class Q_WAYLAND_COMPOSITOR_EXPORT BufferManager : public QObject
{
public:
    BufferManager(QWaylandCompositor *compositor);
    ClientBuffer *getBuffer(struct ::wl_resource *buffer_resource);
private:
    friend struct buffer_manager_destroy_listener;
    static void destroy_listener_callback(wl_listener *listener, void *data);

    QHash<struct ::wl_resource *, ClientBuffer*> m_buffers;
    QWaylandCompositor *m_compositor = nullptr;
};

}
QT_END_NAMESPACE

#endif // QWLBUFFERMANAGER_H
