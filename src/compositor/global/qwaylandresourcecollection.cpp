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

#include "qwaylandresourcecollection.h"

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

struct wl_resource *resourceForClient(const struct wl_list *list, struct wl_client *client)
{
    struct wl_resource *resource;
    wl_list_for_each(resource, list, link) {
        if (resource->client == client) {
            return resource;
        }
    }
    return 0;
}

ResourceCollection::ResourceCollection()
{
    wl_list_init(&client_resources);
}

ResourceCollection::~ResourceCollection()
{

}

void ResourceCollection::registerResource(struct wl_resource *resource)
{
    wl_list_insert(&client_resources,&resource->link);
    struct wl_listener *listener = new struct wl_listener;
    listener->notify = ResourceCollection::destroy_listener_notify;
    wl_signal_add(&resource->destroy_signal, listener);
}

struct wl_resource *ResourceCollection::resourceForClient(wl_client *client) const
{
    return QtWayland::resourceForClient(&client_resources, client);
}

bool ResourceCollection::resourceListIsEmpty() const
{
    return wl_list_empty(const_cast<struct wl_list *>(&client_resources));
}

void ResourceCollection::destroy_listener_notify(struct wl_listener *listener, void *data)
{
    struct wl_resource *resource = reinterpret_cast<struct wl_resource *>(data);
    wl_list_remove(&resource->link);
    delete listener;
}

}

QT_END_NAMESPACE
