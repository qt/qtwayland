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

#include "wldatadevicemanager.h"

#include "wldatadevice.h"
#include "wldatasource.h"
#include "wlinputdevice.h"
#include "wlcompositor.h"
#include "wldataoffer.h"

#include <QtCore/QDebug>

namespace Wayland {

DataDeviceManager::DataDeviceManager(Compositor *compositor)
    : m_compositor(compositor)
    , m_current_selection_source(0)
{
    wl_display_add_global(compositor->wl_display(), &wl_data_device_manager_interface, base(), DataDeviceManager::bind_func_drag);
}

void DataDeviceManager::setCurrentSelectionSource(DataSource *source)
{
    if (m_current_selection_source == source)
        return;

    if (m_current_selection_source) {
        if (m_current_selection_source->time() >= source->time()) {
            qDebug() << "Trying to set older selection";
            return;
        }
    }
    m_current_selection_source = source;
}

DataSource *DataDeviceManager::currentSelectionSource()
{
    return m_current_selection_source;
}

struct wl_display *DataDeviceManager::display() const
{
    return m_compositor->wl_display();
}

void DataDeviceManager::bind_func_drag(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_client_add_object(client,&wl_data_device_manager_interface,&drag_interface,id,data);
}

void DataDeviceManager::bind_func_data(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
}

void DataDeviceManager::get_data_device(struct wl_client *client,
                      struct wl_resource *data_device_manager_resource,
                      uint32_t id,
                      struct wl_resource *input_device_resource)
{
    DataDeviceManager *data_device_manager = static_cast<DataDeviceManager *>(data_device_manager_resource->data);
    InputDevice *input_device = reinterpret_cast<InputDevice *>(input_device_resource->data);
    input_device->clientRequestedDataDevice(data_device_manager,client,id);
}

void DataDeviceManager::create_data_source(struct wl_client *client,
                               struct wl_resource *data_device_manager_resource,
                               uint32_t id)
{
    Q_UNUSED(data_device_manager_resource);
    new DataSource(client,id, Compositor::currentTimeMsecs());
}

struct wl_data_device_manager_interface DataDeviceManager::drag_interface = {
    DataDeviceManager::create_data_source,
    DataDeviceManager::get_data_device
};

} //namespace
