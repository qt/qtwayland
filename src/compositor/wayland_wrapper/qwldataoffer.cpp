/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwldataoffer_p.h"

#include "qwldatadevice_p.h"

#include <wayland-server.h>

#include <sys/time.h>
#include <unistd.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland
{

DataOffer::DataOffer(DataSource *data_source)
    : m_data_source(data_source)
{

}

DataOffer::~DataOffer()
{
}

struct wl_resource *DataOffer::addDataDeviceResource(struct wl_resource *data_device_resource)
{
    if (resourceForClient(data_device_resource->client)) {
        qDebug() << "This should not happen, the client tries to add twice to a data offer";
        return 0;
    }
    struct wl_resource *new_object =
             wl_client_new_object(data_device_resource->client,&wl_data_offer_interface,&data_interface,this);
    wl_data_device_send_data_offer(data_device_resource, new_object);

    registerResource(new_object);
    QList<QByteArray> offer_list = m_data_source->offerList();
    for (int i = 0; i < offer_list.size(); i++) {
        wl_data_offer_send_offer(new_object, offer_list.at(i).constData());
    }
    return new_object;
}

const struct wl_data_offer_interface DataOffer::data_interface = {
    DataOffer::accept,
    DataOffer::receive,
    DataOffer::destroy
};

void DataOffer::accept(wl_client *client, wl_resource *resource, uint32_t time, const char *type)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(time);
    Q_UNUSED(type);
}

void DataOffer::receive(wl_client *client, wl_resource *resource, const char *mime_type, int32_t fd)
{
    Q_UNUSED(client);

    DataOffer *offer = static_cast<DataOffer *>(resource->data);
    offer->m_data_source->postSendEvent(mime_type,fd);
    close(fd);
}

void DataOffer::destroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    DataOffer *data_offer = static_cast<DataOffer *>(resource->data);

    if (data_offer->resourceListIsEmpty()) {
        delete data_offer;
    }
}

}

QT_END_NAMESPACE
