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

#include "qwldatadevice_p.h"

#include "qwldatasource_p.h"
#include "qwldataoffer_p.h"
#include "qwldatadevicemanager_p.h"

#include <stdlib.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

void DataDevice::start_drag(struct wl_client *client,
                   struct wl_resource *resource,
                   struct wl_resource *source,
                   struct wl_resource *surface,
                   struct wl_resource *icon,
                   uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(icon);
    Q_UNUSED(time);
    DataDevice *data_device = static_cast<DataDevice *>(resource->data);
    DataSource *data_source = static_cast<DataSource *>(source->data);
    Q_UNUSED(data_device);
    Q_UNUSED(data_source);
}

void DataDevice::set_selection(struct wl_client *client,
                      struct wl_resource *data_device_resource,
                      struct wl_resource *source,
                      uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(time);
    DataDevice *data_device = static_cast<DataDevice *>(data_device_resource->data);
    DataSource *data_source = static_cast<DataSource *>(source->data);

    data_device->m_data_device_manager->setCurrentSelectionSource(data_source);

}

const struct wl_data_device_interface DataDevice::data_device_interface = {
    DataDevice::start_drag,
    DataDevice::set_selection
};

DataDevice::DataDevice(DataDeviceManager *data_device_manager, struct wl_client *client, uint32_t id)
    : m_data_device_manager(data_device_manager)
    , m_sent_selection_time(0)
{

    //static int i = 0;
    //qDebug() << "data device" << ++i;
    m_data_device_resource =
            wl_client_add_object(client,&wl_data_device_interface,&data_device_interface,id, this);
}

void DataDevice::sendSelectionFocus()
{
    if (m_data_device_manager->offerFromCompositorToClient(m_data_device_resource))
        return;

    DataSource *source = m_data_device_manager->currentSelectionSource();
    if (!source || !source->client()) {
        m_data_device_manager->offerRetainedSelection(m_data_device_resource);
        return;
    }
    if (source->time() > m_sent_selection_time) { //this makes sure we don't resend
        if (source->client() != m_data_device_resource->client) { //don't send selection to the client that owns the selection
            DataOffer *data_offer = source->dataOffer();
            wl_resource *client_resource =
                    data_offer->addDataDeviceResource(m_data_device_resource);
            //qDebug() << "sending data_offer for source" << source;
            wl_data_device_send_selection(m_data_device_resource,client_resource);
            m_sent_selection_time = source->time();
        }
    }
}

struct wl_resource *DataDevice::dataDeviceResource() const
{
    return m_data_device_resource;
}

}

QT_END_NAMESPACE
