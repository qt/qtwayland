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

#include "wlinputdevice.h"

#include "wlshmbuffer.h"
#include "wlcompositor.h"
#include "wldatadevice.h"
#include "wlsurface.h"

#include <QtCore/QDebug>

#include "waylandcompositor.h"

namespace Wayland {

static ShmBuffer *currentCursor;

InputDevice::InputDevice(Compositor *compositor)
{
    wl_input_device_init(base());
    wl_display_add_global(compositor->wl_display(),&wl_input_device_interface,this,InputDevice::bind_func);
}

void InputDevice::clientRequestedDataDevice(DataDeviceManager *data_device_manager, struct wl_client *client, uint32_t id)
{
    for (int i = 0; i < m_data_devices.size(); i++) {
        struct wl_resource *data_device_resource =
                m_data_devices.at(i)->dataDeviceResource();
        if (data_device_resource->client == client) {
            qDebug() << "Client created data device, but already has one; removing the old one!";
            m_data_devices.removeAt(i);
            free(data_device_resource);
            break;
        }
    }
    DataDevice *dataDevice = new DataDevice(data_device_manager,client,id);
    m_data_devices.append(dataDevice);
    qDebug("created datadevice %p resource %p", dataDevice, dataDevice->dataDeviceResource());
}

void InputDevice::sendSelectionFocus(Surface *surface)
{
    if (!surface)
        return;
    DataDevice *device = dataDevice(surface->base()->resource.client);
    if (device) {
        device->sendSelectionFocus();
    }
}

DataDevice *InputDevice::dataDevice(struct wl_client *client) const
{
    for (int i = 0; i < m_data_devices.size();i++) {
        if (m_data_devices.at(i)->dataDeviceResource()->client == client) {
            return m_data_devices.at(i);
        }
    }
    return 0;
}

void InputDevice::bind_func(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    struct wl_resource *resource = wl_client_add_object(client,&wl_input_device_interface ,&input_device_interface,id,data);

    struct wl_input_device *input_device = static_cast<struct wl_input_device *>(data);
    resource->destroy = destroy_resource;
    wl_list_insert(&input_device->resource_list,&resource->link);
}

void InputDevice::input_device_attach(struct wl_client *client,
                         struct wl_resource *device_resource,
                         uint32_t time,
                         struct wl_resource *buffer_resource, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    Q_UNUSED(time);

    struct wl_input_device *device_base = reinterpret_cast<struct wl_input_device *>(device_resource);
    struct wl_buffer *buffer = reinterpret_cast<struct wl_buffer *>(buffer_resource);
    qDebug() << "Client input device attach" << client << buffer << x << y;

//    Compositor *compositor = wayland_cast<Compositor *>(device_base->compositor);
    ShmBuffer *shmBuffer = static_cast<ShmBuffer *>(buffer->user_data);
    if (shmBuffer) {
//        compositor->qtCompositor()->changeCursor(shmBuffer->image(), x, y);
        currentCursor = shmBuffer;
    }
}

const struct wl_input_device_interface InputDevice::input_device_interface = {
    InputDevice::input_device_attach,
};

void InputDevice::destroy_resource(wl_resource *resource)
{
    qDebug() << "input device resource destroy" << resource;
    InputDevice *input_device = static_cast<InputDevice *>(resource->data);
    if (input_device->base()->keyboard_focus_resource == resource) {
        input_device->base()->keyboard_focus_resource = 0;
    }
    if (input_device->base()->pointer_focus_resource == resource) {
        input_device->base()->pointer_focus_resource = 0;
    }

    input_device->m_data_devices.clear();

    wl_list_remove(&resource->link);

    free(resource);
}

}
