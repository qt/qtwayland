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

#include "mockcompositor.h"
#include "mocksurface.h"

namespace Impl {

void Compositor::destroyInputResource(wl_resource *resource)
{
    Compositor *compositor = static_cast<Compositor *>(resource->data);
    wl_keyboard *keyboard = &compositor->m_keyboard;
    wl_pointer *pointer = &compositor->m_pointer;

    if (keyboard->focus_resource == resource)
        keyboard->focus_resource = 0;
    if (pointer->focus_resource == resource)
        pointer->focus_resource = 0;

    wl_list_remove(&resource->link);

    free(resource);
}

static void destroyInputDevice(wl_resource *resource)
{
    wl_list_remove(&resource->link);
    free(resource);
}

void pointer_attach(wl_client *client,
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

void Compositor::get_pointer(wl_client *client,
                             wl_resource *resource,
                             uint32_t id)
{
    static const struct wl_pointer_interface pointer_interface = {
        pointer_attach
    };
    Compositor *compositor = static_cast<Compositor *>(resource->data);
    wl_pointer *pointer = &compositor->m_pointer;
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_pointer_interface,
                                                       &pointer_interface,
                                                       id,
                                                       pointer);
    wl_list_insert(&pointer->resource_list, &clientResource->link);
    clientResource->destroy = destroyInputDevice;
}

void Compositor::get_keyboard(wl_client *client,
                              wl_resource *resource,
                              uint32_t id)
{
    Compositor *compositor = static_cast<Compositor *>(resource->data);
    wl_keyboard *keyboard = &compositor->m_keyboard;
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_keyboard_interface,
                                                       0,
                                                       id,
                                                       keyboard);
    wl_list_insert(&keyboard->resource_list, &clientResource->link);
    clientResource->destroy = destroyInputDevice;
}

void Compositor::get_touch(wl_client *client,
                           wl_resource *resource,
                           uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(id);
}

void Compositor::bindSeat(wl_client *client, void *compositorData, uint32_t version, uint32_t id)
{
    static const struct wl_seat_interface seatInterface = {
        get_pointer,
        get_keyboard,
        get_touch
    };

    Q_UNUSED(version);
    wl_resource *resource = wl_client_add_object(client, &wl_seat_interface, &seatInterface, id, compositorData);
    resource->destroy = destroyInputResource;

    Compositor *compositor = static_cast<Compositor *>(compositorData);
    wl_list_insert(&compositor->m_seat.base_resource_list, &resource->link);

    wl_seat_send_capabilities(resource, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
}

static wl_surface *resolveSurface(const QVariant &v)
{
    QSharedPointer<MockSurface> mockSurface = v.value<QSharedPointer<MockSurface> >();
    Surface *surface = mockSurface ? mockSurface->handle() : 0;
    return surface ? surface->base() : 0;
}

void Compositor::setKeyboardFocus(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_keyboard_set_focus(&compositor->m_keyboard, resolveSurface(parameters.first()));
}

void Compositor::sendMousePress(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    QPoint pos = parameters.last().toPoint();
    wl_pointer_set_focus(&compositor->m_pointer, surface,
                         wl_fixed_from_int(pos.x()), wl_fixed_from_int(pos.y()));
    wl_pointer_send_motion(compositor->m_pointer.focus_resource, compositor->time(),
                           wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
    wl_pointer_send_button(compositor->m_pointer.focus_resource,
                           compositor->nextSerial(), compositor->time(), 0x110, 1);
}

void Compositor::sendMouseRelease(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    wl_pointer_send_button(compositor->m_pointer.focus_resource,
                           compositor->nextSerial(), compositor->time(), 0x110, 0);
}

void Compositor::sendKeyPress(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    wl_keyboard_send_key(compositor->m_keyboard.focus_resource,
                         compositor->nextSerial(), compositor->time(), parameters.last().toUInt() - 8, 1);
}

void Compositor::sendKeyRelease(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    wl_surface *surface = resolveSurface(parameters.first());
    if (!surface)
        return;

    wl_keyboard_send_key(compositor->m_keyboard.focus_resource,
                         compositor->nextSerial(), compositor->time(), parameters.last().toUInt() - 8, 0);
}

}
