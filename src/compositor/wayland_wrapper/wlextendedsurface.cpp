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

#include "wlextendedsurface.h"

#include "wlcompositor.h"
#include "wlsurface.h"

namespace Wayland {

SurfaceExtensionGlobal::SurfaceExtensionGlobal(Compositor *compositor)
    : m_compositor(compositor)
{
    wl_display_add_global(m_compositor->wl_display(),
                          &wl_surface_extension_interface,
                          this,
                          SurfaceExtensionGlobal::bind_func);
}

void SurfaceExtensionGlobal::bind_func(struct wl_client *client, void *data,
                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_client_add_object(client, &wl_surface_extension_interface,&surface_extension_interface,id,data);
}

const struct wl_surface_extension_interface SurfaceExtensionGlobal::surface_extension_interface = {
    SurfaceExtensionGlobal::get_extended_surface
};

void SurfaceExtensionGlobal::get_extended_surface(struct wl_client *client,
                             struct wl_resource *surface_extension_resource,
                             uint32_t id,
                             struct wl_resource *surface_resource)
{
    Q_UNUSED(surface_extension_resource);
    Surface *surface = reinterpret_cast<Surface *>(surface_resource);
    new ExtendedSurface(client,id,surface);
}

ExtendedSurface::ExtendedSurface(struct wl_client *client, uint32_t id, Surface *surface)
    : m_surface(surface)
{
    Q_ASSERT(surface->extendedSurface() == 0);
    surface->setExtendedSurface(this);
    m_extended_surface_resource = wl_client_add_object(client,
                                                       &wl_extended_surface_interface,
                                                       &extended_surface_interface,
                                                       id,
                                                       this);
}

ExtendedSurface::~ExtendedSurface()
{

}

void ExtendedSurface::sendGenericProperty(const char *name, const QVariant &variant)
{
    QByteArray byteValue;
    QDataStream ds(&byteValue, QIODevice::WriteOnly);
    ds << variant;
    wl_array data;
    data.size = byteValue.size();
    data.data = (void*) byteValue.constData();
    data.alloc = 0;
    wl_resource_post_event(m_extended_surface_resource,WL_EXTENDED_SURFACE_SET_GENERIC_PROPERTY, name,&data);

}

void ExtendedSurface::sendOnScreenVisibllity(bool visible)
{
    int32_t visibleInt = visible;
    wl_resource_post_event(m_extended_surface_resource,WL_EXTENDED_SURFACE_ONSCREEN_VISIBILITY,visibleInt);
}


void ExtendedSurface::update_generic_property(wl_client *client, wl_resource *extended_surface_resource, const char *name, wl_array *value)
{
    Q_UNUSED(client);
    ExtendedSurface *extended_surface = static_cast<ExtendedSurface *>(extended_surface_resource->data);
    QVariant variantValue;
    QByteArray byteValue((const char*)value->data, value->size);
    QDataStream ds(&byteValue, QIODevice::ReadOnly);
    ds >> variantValue;
    extended_surface->m_surface->setWindowProperty(QString::fromLatin1(name),variantValue,false);

}

const struct wl_extended_surface_interface ExtendedSurface::extended_surface_interface = {
    ExtendedSurface::update_generic_property
};

}
