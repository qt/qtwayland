// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
