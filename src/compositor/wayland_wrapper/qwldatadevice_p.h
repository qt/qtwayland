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

#ifndef WLDATADEVICE_H
#define WLDATADEVICE_H

#include <private/qwldatadevicemanager_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class DataSource;
class DataDeviceManager;

class DataDevice
{
public:
    DataDevice(DataDeviceManager *data_device_manager, struct wl_client *client, uint32_t id);

    void createAndSetSelectionSource(struct wl_client *client, uint32_t id, const char *name, uint32_t time);
    void sendSelectionFocus();

    struct wl_resource *dataDeviceResource() const;

    struct wl_display *display() const { return m_data_device_manager->display(); }
private:
    DataDeviceManager *m_data_device_manager;
    uint32_t m_sent_selection_time;
    struct wl_resource *m_data_device_resource;

    static const struct wl_data_device_interface data_device_interface;
    static void start_drag(struct wl_client *client,
                       struct wl_resource *resource,
                       struct wl_resource *source,
                       struct wl_resource *surface,
                       struct wl_resource *icon,
                       uint32_t time);
    static void set_selection(struct wl_client *client,
                          struct wl_resource *resource,
                          struct wl_resource *source,
                          uint32_t time);
};

}

QT_END_NAMESPACE

#endif // WLDATADEVICE_H
