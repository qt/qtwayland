/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
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
    Surface *surface = static_cast<Surface *>(surfaceResource->data);
    wl_resource_destroy(surfaceResource, surface->compositor()->time());
}

void surface_attach(wl_client *client, wl_resource *surface,
                    wl_resource *buffer, int x, int y)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(buffer);
    Q_UNUSED(x);
    Q_UNUSED(y);
    //resolve<Surface>(surface)->attach(buffer ? reinterpret_cast<wl_buffer *>(buffer->data) : 0);
}

void surface_damage(wl_client *client, wl_resource *surface,
                    int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    //resolve<Surface>(surface)->damage(QRect(x, y, width, height));
}

void surface_frame(wl_client *client,
                   wl_resource *surface,
                   uint32_t callback)
{
    Q_UNUSED(client);
    Q_UNUSED(surface);
    Q_UNUSED(callback);
//    Surface *surface = resolve<Surface>(resource);
//    wl_resource *frame_callback = wl_client_add_object(client, &wl_callback_interface, 0, callback, surface);
//    wl_list_insert(&surface->m_frame_callback_list, &frame_callback->link);
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
