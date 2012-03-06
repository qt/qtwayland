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

#include "mockcompositor.h"
#include "mocksurface.h"

namespace Impl {

void Compositor::destroyInputResource(wl_resource *resource)
{
    Compositor *compositor = static_cast<Compositor *>(resource->data);
    wl_input_device *input = &compositor->m_input;

    if (input->keyboard_focus_resource == resource)
        input->keyboard_focus_resource = 0;
    if (input->pointer_focus_resource == resource)
        input->pointer_focus_resource = 0;

    wl_list_remove(&resource->link);

    free(resource);
}

void input_device_attach(wl_client *client,
                         wl_resource *device_resource,
                         uint32_t time,
                         wl_resource *buffer_resource, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    Q_UNUSED(device_resource);
    Q_UNUSED(time);
    Q_UNUSED(buffer_resource);
    Q_UNUSED(QPoint(x, y));
}

void Compositor::bindInput(wl_client *client, void *compositorData, uint32_t version, uint32_t id)
{
    static const struct wl_input_device_interface inputDeviceInterface = {
        input_device_attach,
    };

    Q_UNUSED(version);
    wl_resource *resource = wl_client_add_object(client, &wl_input_device_interface, &inputDeviceInterface, id, compositorData);
    resource->destroy = destroyInputResource;

    Compositor *compositor = static_cast<Compositor *>(compositorData);
    wl_list_insert(&compositor->m_input.resource_list, &resource->link);
}

static wl_surface *resolveSurface(const QVariant &v)
{
    QSharedPointer<MockSurface> mockSurface = v.value<QSharedPointer<MockSurface> >();
    Surface *surface = mockSurface ? mockSurface->handle() : 0;
    return surface ? surface->handle() : 0;
}

void Compositor::setKeyboardFocus(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_input_device_set_keyboard_focus(&compositor->m_input, resolveSurface(parameters.first()), compositor->time());
}

void Compositor::sendMousePress(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    QPoint pos = parameters.last().toPoint();
    wl_input_device_set_pointer_focus(&compositor->m_input, surface, compositor->time(), pos.x(), pos.y());
    wl_input_device_send_motion(compositor->m_input.pointer_focus_resource, compositor->time(), pos.x(), pos.y());
    wl_input_device_send_button(compositor->m_input.pointer_focus_resource, compositor->time(), 0x110, 1);
}

void Compositor::sendMouseRelease(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    wl_input_device_send_button(compositor->m_input.pointer_focus_resource, compositor->time(), 0x110, 0);
}

}

