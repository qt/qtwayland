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

#ifndef WLEXTENDEDSURFACE_H
#define WLEXTENDEDSURFACE_H

#include <wayland-server.h>
#include "wayland-surface-extension-server-protocol.h"

#include <private/qwlsurface_p.h>
#include <QtCompositor/qwaylandsurface.h>

#include <QtCore/QVariant>
#include <QtCore/QLinkedList>

QT_BEGIN_NAMESPACE

class QWaylandSurface;

namespace QtWayland {

class Compositor;

class SurfaceExtensionGlobal
{
public:
    SurfaceExtensionGlobal(Compositor *compositor);

private:
    Compositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
    static void get_extended_surface(struct wl_client *client,
                                 struct wl_resource *resource,
                                 uint32_t id,
                                 struct wl_resource *surface);
    static const struct wl_surface_extension_interface surface_extension_interface;

};

class ExtendedSurface
{
public:
    ExtendedSurface(struct wl_client *client, uint32_t id, Surface *surface);
    ~ExtendedSurface();

    void sendGenericProperty(const QString &name, const QVariant &variant);
    void sendOnScreenVisibility(bool visible);

    void setSubSurface(ExtendedSurface *subSurface,int x, int y);
    void removeSubSurface(ExtendedSurface *subSurfaces);
    ExtendedSurface *parent() const;
    void setParent(ExtendedSurface *parent);
    QLinkedList<QWaylandSurface *> subSurfaces() const;

    Qt::ScreenOrientation contentOrientation() const;

    QWaylandSurface::WindowFlags windowFlags() const { return m_windowFlags; }

    qint64 processId() const;
    void setProcessId(qint64 processId);

    QVariantMap windowProperties() const;
    QVariant windowProperty(const QString &propertyName) const;
    void setWindowProperty(const QString &name, const QVariant &value, bool writeUpdateToClient = true);

private:
    struct wl_resource *m_extended_surface_resource;
    Surface *m_surface;

    Qt::ScreenOrientation m_contentOrientation;

    QWaylandSurface::WindowFlags m_windowFlags;

    QByteArray m_authenticationToken;
    QVariantMap m_windowProperties;


    static void update_generic_property(struct wl_client *client,
                                    struct wl_resource *resource,
                                    const char *name,
                                    struct wl_array *value);

    static void set_content_orientation(struct wl_client *client,
                                        struct wl_resource *resource,
                                        int32_t orientation);

    static void set_window_flags(struct wl_client *client,
                                 struct wl_resource *resource,
                                 int32_t flags);
    void setWindowFlags(QWaylandSurface::WindowFlags flags);

    static const struct wl_extended_surface_interface extended_surface_interface;
};

}

QT_END_NAMESPACE

#endif // WLEXTENDEDSURFACE_H
