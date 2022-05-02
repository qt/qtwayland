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

#include "qwldatasource_p.h"
#include "qwldataoffer_p.h"
#include "qwldatadevice_p.h"
#include "qwldatadevicemanager_p.h"
#include <QtWaylandCompositor/private/qwaylandutils_p.h>

#include <unistd.h>
#include <QtWaylandCompositor/private/wayland-wayland-server-protocol.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

DataSource::DataSource(struct wl_client *client, uint32_t id, uint32_t time)
    : QtWaylandServer::wl_data_source(client, id, 1)
    , m_time(time)
{
}

DataSource::~DataSource()
{
    if (m_manager)
        m_manager->sourceDestroyed(this);
    if (m_device)
        m_device->sourceDestroyed(this);
}

uint32_t DataSource::time() const
{
    return m_time;
}

QList<QString> DataSource::mimeTypes() const
{
    return m_mimeTypes;
}

void DataSource::accept(const QString &mimeType)
{
    send_target(mimeType);
}

void DataSource::send(const QString &mimeType, int fd)
{
    send_send(mimeType, fd);
    close(fd);
}

void DataSource::cancel()
{
    send_cancelled();
}

void DataSource::setManager(DataDeviceManager *mgr)
{
    m_manager = mgr;
}

void DataSource::setDevice(DataDevice *device)
{
    m_device = device;
}

DataSource *DataSource::fromResource(struct ::wl_resource *resource)
{
    return QtWayland::fromResource<DataSource *>(resource);
}

void DataSource::data_source_offer(Resource *, const QString &mime_type)
{
    m_mimeTypes.append(mime_type);
}

void DataSource::data_source_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void DataSource::data_source_destroy_resource(QtWaylandServer::wl_data_source::Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

}

QT_END_NAMESPACE
