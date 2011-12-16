/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "wldatasource.h"
#include "wldataoffer.h"
#include "wldatadevicemanager.h"
#include "wlcompositor.h"
#include <wayland-server.h>
#include <QtCore/QDebug>

namespace Wayland {

DataSource::DataSource(struct wl_client *client, uint32_t id, uint32_t time)
    : m_time(time)
{
    m_data_source_resource = wl_client_add_object(client, &wl_data_source_interface, &DataSource::data_source_interface,id,this);
    m_data_source_resource->destroy = resource_destroy;
    m_data_offer = new DataOffer(this);
    m_manager = 0;
}

DataSource::~DataSource()
{
    qDebug() << "destroying source";
    if (m_manager)
        m_manager->sourceDestroyed(this);
    wl_resource_destroy(m_data_source_resource,Compositor::currentTimeMsecs());
}

void DataSource::resource_destroy(wl_resource *resource)
{
    DataSource *source = static_cast<DataSource *>(resource->data);
    if (source && source->m_data_source_resource == resource)
        source->m_data_source_resource = 0;
    free(resource);
}

uint32_t DataSource::time() const
{
    return m_time;
}

QList<QByteArray> DataSource::offerList() const
{
    return m_offers;
}

struct wl_data_source_interface DataSource::data_source_interface = {
    DataSource::offer,
    DataSource::destroy
};

void DataSource::offer(struct wl_client *client,
              struct wl_resource *resource,
              const char *type)
{
    Q_UNUSED(client);
    qDebug() << "received offer" << type;
    static_cast<DataSource *>(resource->data)->m_offers.append(type);
}

void DataSource::destroy(struct wl_client *client,
                struct wl_resource *resource)
{
    DataSource *self = static_cast<DataSource *>(resource->data);
    delete self;
}

DataOffer * DataSource::dataOffer() const
{
    return m_data_offer;
}

void DataSource::postSendEvent(const QByteArray &mimeType, int fd)
{
    if (m_data_source_resource) {
        wl_resource_post_event(m_data_source_resource,WL_DATA_SOURCE_SEND,mimeType.constData(),fd);
    }
}

struct wl_client *DataSource::client() const
{
    if (m_data_source_resource)
        return m_data_source_resource->client;
    return 0;
}

void DataSource::setManager(DataDeviceManager *mgr)
{
    m_manager = mgr;
}

}
