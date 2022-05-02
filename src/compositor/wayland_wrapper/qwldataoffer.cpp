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

#include "qwldataoffer_p.h"

#include "qwldatadevice_p.h"
#include "qwldatasource_p.h"

#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace QtWayland
{

DataOffer::DataOffer(DataSource *dataSource, QtWaylandServer::wl_data_device::Resource *target)
    : QtWaylandServer::wl_data_offer(target->client(), 0, 1)
    , m_dataSource(dataSource)
{
    // FIXME: connect to dataSource and reset m_dataSource on destroy
    target->data_device_object->send_data_offer(target->handle, resource()->handle);
    const auto mimeTypes = dataSource->mimeTypes();
    for (const QString &mimeType : mimeTypes) {
        send_offer(mimeType);
    }
}

DataOffer::~DataOffer()
{
}

void DataOffer::data_offer_accept(Resource *resource, uint32_t serial, const QString &mimeType)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    if (m_dataSource)
        m_dataSource->accept(mimeType);
}

void DataOffer::data_offer_receive(Resource *resource, const QString &mimeType, int32_t fd)
{
    Q_UNUSED(resource);
    if (m_dataSource)
        m_dataSource->send(mimeType, fd);
    else
        close(fd);
}

void DataOffer::data_offer_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void DataOffer::data_offer_destroy_resource(Resource *)
{
    delete this;
}

}

QT_END_NAMESPACE
