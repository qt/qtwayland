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

#ifndef WAYLAND_OBJECT_H
#define WAYLAND_OBJECT_H

#include <wayland-server.h>

#include <string.h>

namespace Wayland {

template <typename T>
class Object
{
public:
    Object() { memset(&m_waylandObject, 0, sizeof(T)); }

    const T *base() const { return &m_waylandObject; }
    T *base() { return &m_waylandObject; }

private:
    T m_waylandObject;
};

template <typename To, typename From>
To wayland_cast(From *from)
{
    Object<From> *object = reinterpret_cast<Object<From> *>(from);
    return static_cast<To>(object);
}

template <typename Implementation>
void addClientResource(struct wl_client *client,
                       struct wl_resource *resource,
                       int id, const struct wl_interface *interface,
                       Implementation implementation,
                       void (*destroy)(struct wl_resource *resource))
{
    resource->object.id = id;
    resource->object.interface = interface;
    resource->object.implementation = (void (**)(void))implementation;
    resource->destroy = destroy;

    wl_client_add_resource(client, resource);
}

}

#endif //WAYLAND_OBJECT_H
