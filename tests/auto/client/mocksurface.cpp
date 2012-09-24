/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mocksurface.h"
#include "mockcompositor.h"

namespace Impl {

void destroy_surface(wl_resource *resource)
{
    Surface *surface = static_cast<Surface *>(resource->data);
    surface->compositor()->removeSurface(surface);
    delete surface;
}

static void surface_destroy(wl_client *, wl_resource *surfaceResource)
{
    wl_resource_destroy(surfaceResource);
}

void surface_attach(wl_client *client, wl_resource *surfaceResource,
                    wl_resource *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);

    Surface *surface = static_cast<Surface *>(surfaceResource->data);
    surface->m_buffer = buffer ? static_cast<wl_buffer *>(buffer->data) : 0;

    if (!buffer)
        surface->m_mockSurface->image = QImage();
}

void surface_damage(wl_client *client, wl_resource *surfaceResource,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);

    Surface *surface = static_cast<Surface *>(surfaceResource->data);
    wl_buffer *buffer = surface->m_buffer;

    if (!buffer)
        return;

    if (wl_buffer_is_shm(buffer)) {
        int stride = wl_shm_buffer_get_stride(buffer);
        uint format = wl_shm_buffer_get_format(buffer);
        (void) format;
        void *data = wl_shm_buffer_get_data(buffer);
        const uchar *char_data = static_cast<const uchar *>(data);
        QImage img(char_data, buffer->width, buffer->height, stride, QImage::Format_ARGB32_Premultiplied);
        surface->m_mockSurface->image = img;
    }

    wl_resource *frameCallback;
    wl_list_for_each(frameCallback, &surface->m_frameCallbackList, link) {
        wl_callback_send_done(frameCallback, surface->m_compositor->time());
        wl_resource_destroy(frameCallback);
    }

    wl_list_init(&surface->m_frameCallbackList);
}

void surface_frame(wl_client *client,
                   wl_resource *surfaceResource,
                   uint32_t callback)
{
    Surface *surface = static_cast<Surface *>(surfaceResource->data);
    wl_resource *frameCallback = wl_client_add_object(client, &wl_callback_interface, 0, callback, surface);
    wl_list_insert(&surface->m_frameCallbackList, &frameCallback->link);
}

void surface_set_opaque_region(wl_client *client, wl_resource *surfaceResource,
                               wl_resource *region)
{
    Q_UNUSED(client);
    Q_UNUSED(surfaceResource);
    Q_UNUSED(region);
}

void surface_set_input_region(wl_client *client, wl_resource *surfaceResource,
                              wl_resource *region)
{
    Q_UNUSED(client);
    Q_UNUSED(surfaceResource);
    Q_UNUSED(region);
}

Surface::Surface(wl_client *client, uint32_t id, Compositor *compositor)
    : m_surface(wl_surface())
    , m_compositor(compositor)
    , m_mockSurface(new MockSurface(this))
{
    static const struct wl_surface_interface surfaceInterface = {
        surface_destroy,
        surface_attach,
        surface_damage,
        surface_frame,
        surface_set_opaque_region,
        surface_set_input_region
    };

    m_surface.resource.object.id = id;
    m_surface.resource.object.interface = &wl_surface_interface;
    m_surface.resource.object.implementation = (Implementation)&surfaceInterface;
    m_surface.resource.data = this;
    m_surface.resource.destroy = destroy_surface;

    wl_client_add_resource(client, &m_surface.resource);

    wl_list_init(&m_frameCallbackList);
}

Surface::~Surface()
{
    m_mockSurface->m_surface = 0;
}

}

MockSurface::MockSurface(Impl::Surface *surface)
    : m_surface(surface)
{
}
