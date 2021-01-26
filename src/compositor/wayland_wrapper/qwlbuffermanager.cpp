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

#include "qwlbuffermanager_p.h"
#include <QWaylandCompositor>
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

BufferManager::BufferManager(QWaylandCompositor *compositor)
    : QObject(compositor)
    , m_compositor(compositor)
{

}

struct buffer_manager_destroy_listener : wl_listener
{
    buffer_manager_destroy_listener()
    {
        notify = BufferManager::destroy_listener_callback;
        wl_list_init(&this->link);
    }

    BufferManager *d = nullptr;
};

ClientBuffer *BufferManager::getBuffer(wl_resource *buffer_resource)
{
    if (!buffer_resource)
        return nullptr;

    auto it = m_buffers.find(buffer_resource);
    if (it != m_buffers.end())
        return it.value();

    auto bufferIntegration = QWaylandCompositorPrivate::get(m_compositor)->clientBufferIntegration();
    ClientBuffer *newBuffer = nullptr;
    if (bufferIntegration)
        newBuffer = bufferIntegration->createBufferFor(buffer_resource);
    if (!newBuffer)
        newBuffer = new SharedMemoryBuffer(buffer_resource);
    m_buffers[buffer_resource] = newBuffer;

    auto *destroy_listener = new buffer_manager_destroy_listener;
    destroy_listener->d = this;
    wl_resource_add_destroy_listener(buffer_resource, destroy_listener);
    return newBuffer;
}


void BufferManager::destroy_listener_callback(wl_listener *listener, void *data)
{
    buffer_manager_destroy_listener *destroy_listener = static_cast<buffer_manager_destroy_listener *>(listener);
    BufferManager *self = destroy_listener->d;
    struct ::wl_resource *buffer = static_cast<struct ::wl_resource *>(data);

    wl_list_remove(&destroy_listener->link);
    delete destroy_listener;

    Q_ASSERT(self);
    Q_ASSERT(buffer);

    ClientBuffer *clientBuffer = self->m_buffers.take(buffer);

    if (!clientBuffer)
        return;

    clientBuffer->setDestroyed();
}

}
QT_END_NAMESPACE
