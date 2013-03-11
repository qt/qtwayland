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

#include "qwlextendedsurface_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

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
    Surface *surface = resolve<Surface>(surface_resource);
    new ExtendedSurface(client,id,surface);
}

ExtendedSurface::ExtendedSurface(struct wl_client *client, uint32_t id, Surface *surface)
    : m_surface(surface)
    , m_contentOrientation(Qt::PrimaryOrientation)
    , m_windowFlags(0)
{
    Q_ASSERT(surface->extendedSurface() == 0);
    m_extended_surface_resource = wl_client_add_object(client,
                                                       &wl_extended_surface_interface,
                                                       &extended_surface_interface,
                                                       id,
                                                       this);
    surface->setExtendedSurface(this);
}

ExtendedSurface::~ExtendedSurface()
{

}

void ExtendedSurface::sendGenericProperty(const QString &name, const QVariant &variant)
{
    QByteArray byteValue;
    QDataStream ds(&byteValue, QIODevice::WriteOnly);
    ds << variant;
    wl_array data;
    data.size = byteValue.size();
    data.data = (void*) byteValue.constData();
    data.alloc = 0;
    wl_extended_surface_send_set_generic_property(m_extended_surface_resource, qPrintable(name), &data);

}

void ExtendedSurface::sendOnScreenVisibility(bool visible)
{
    int32_t visibleInt = visible;
    wl_extended_surface_send_onscreen_visibility(m_extended_surface_resource, visibleInt);
}


void ExtendedSurface::update_generic_property(wl_client *client, wl_resource *extended_surface_resource, const char *name, wl_array *value)
{
    Q_UNUSED(client);
    ExtendedSurface *extended_surface = static_cast<ExtendedSurface *>(extended_surface_resource->data);
    QVariant variantValue;
    QByteArray byteValue((const char*)value->data, value->size);
    QDataStream ds(&byteValue, QIODevice::ReadOnly);
    ds >> variantValue;
    extended_surface->setWindowProperty(QString::fromLatin1(name),variantValue,false);

}

static Qt::ScreenOrientation screenOrientationFromWaylandOrientation(int32_t orientation)
{
    switch (orientation) {
    case WL_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION: return Qt::PortraitOrientation;
    case WL_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION: return Qt::InvertedPortraitOrientation;
    case WL_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION: return Qt::LandscapeOrientation;
    case WL_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION: return Qt::InvertedLandscapeOrientation;
    default: return Qt::PrimaryOrientation;
    }
}

Qt::ScreenOrientation ExtendedSurface::contentOrientation() const
{
    return m_contentOrientation;
}

void ExtendedSurface::set_content_orientation(struct wl_client *client,
                                              struct wl_resource *extended_surface_resource,
                                              int32_t orientation)
{
    Q_UNUSED(client);
    ExtendedSurface *extended_surface = static_cast<ExtendedSurface *>(extended_surface_resource->data);

    Qt::ScreenOrientation oldOrientation = extended_surface->m_contentOrientation;
    extended_surface->m_contentOrientation = screenOrientationFromWaylandOrientation(orientation);
    if (extended_surface->m_contentOrientation != oldOrientation)
        emit extended_surface->m_surface->waylandSurface()->contentOrientationChanged();
}

void ExtendedSurface::setWindowFlags(QWaylandSurface::WindowFlags flags)
{
    if (flags == m_windowFlags)
        return;
    m_windowFlags = flags;
    emit m_surface->waylandSurface()->windowFlagsChanged(flags);
}

QVariantMap ExtendedSurface::windowProperties() const
{
    return m_windowProperties;
}

QVariant ExtendedSurface::windowProperty(const QString &propertyName) const
{
    QVariantMap props = m_windowProperties;
    return props.value(propertyName);
}

void ExtendedSurface::setWindowProperty(const QString &name, const QVariant &value, bool writeUpdateToClient)
{
    m_windowProperties.insert(name, value);
    m_surface->waylandSurface()->windowPropertyChanged(name,value);
    if (writeUpdateToClient)
        sendGenericProperty(name, value);
}

void ExtendedSurface::set_window_flags(wl_client *client, wl_resource *resource, int32_t flags)
{
    Q_UNUSED(client);
    ExtendedSurface *extended_surface = static_cast<ExtendedSurface *>(resource->data);
    extended_surface->setWindowFlags(QWaylandSurface::WindowFlags(flags));
}

const struct wl_extended_surface_interface ExtendedSurface::extended_surface_interface = {
    ExtendedSurface::update_generic_property,
    ExtendedSurface::set_content_orientation,
    ExtendedSurface::set_window_flags
};

}

QT_END_NAMESPACE
