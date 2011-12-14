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

#ifndef WLDATADEVICEMANAGER_H
#define WLDATADEVICEMANAGER_H

#include "wlcompositor.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QClipboard>


namespace Wayland {

class Compositor;

class DataDevice;
class DataSource;

class DataDeviceManager
{
public:
    DataDeviceManager(Compositor *compositor);

    void setCurrentSelectionSource(DataSource *source);
    DataSource *currentSelectionSource();

    struct wl_display *display() const;
private:
    Compositor *m_compositor;
    QList<DataDevice *> m_data_device_list;

    DataSource *m_current_selection_source;

    static void bind_func_drag(struct wl_client *client, void *data,
                     uint32_t version, uint32_t id);
    static void bind_func_data(struct wl_client *client, void *data,
                                          uint32_t version, uint32_t id);
    static void get_data_device(struct wl_client *client,
                          struct wl_resource *resource,
                          uint32_t id,
                          struct wl_resource *input_device);
    static void create_data_source(struct wl_client *client,
                                   struct wl_resource *resource,
                                   uint32_t id);
    static struct wl_data_device_manager_interface drag_interface;
};

}
#endif // WLDATADEVICEMANAGER_H
