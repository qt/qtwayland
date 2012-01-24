/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#include "wlshellsurface.h"

#include "wlcompositor.h"
#include "wlsurface.h"
#include "wlinputdevice.h"

#include <QtCore/qglobal.h>
#include <QtCore/QDebug>

namespace Wayland {

Shell::Shell()
{
}

void Shell::bind_func(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_client_add_object(client,&wl_shell_interface,&shell_interface,id,data);
}

void Shell::get_shell_surface(struct wl_client *client,
              struct wl_resource *shell_resource,
              uint32_t id,
              struct wl_resource *surface_super)
{
    Q_UNUSED(shell_resource);
    Surface *surface = reinterpret_cast<Surface *>(surface_super);
    new ShellSurface(client,id,surface);
}

const struct wl_shell_interface Shell::shell_interface = {
    Shell::get_shell_surface
};

ShellSurface::ShellSurface(wl_client *client, uint32_t id, Surface *surface)
{
    m_shellSurface = wl_client_add_object(client,&wl_shell_surface_interface,&shell_surface_interface,id,this);
    surface->setShellSurface(this);

}


void ShellSurface::move(struct wl_client *client,
                struct wl_resource *shell_surface_resource,
                struct wl_resource *input_device,
                uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(shell_surface_resource);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
}

void ShellSurface::resize(struct wl_client *client,
                  struct wl_resource *shell_surface_resource,
                  struct wl_resource *input_device_super,
                  uint32_t time,
                  uint32_t edges)
{
    Q_UNUSED(shell_surface_resource);
    Q_UNUSED(client);
    Q_UNUSED(time);
    Q_UNUSED(edges);
    ShellSurface *shell_surface = static_cast<ShellSurface *>(shell_surface_resource->data);
    Q_UNUSED(shell_surface);
    InputDevice *input_device = static_cast<InputDevice *>(input_device_super->data);
    Q_UNUSED(input_device);

}

void ShellSurface::set_toplevel(struct wl_client *client,
                     struct wl_resource *shell_surface_resource)
{
    Q_UNUSED(client);
    Q_UNUSED(shell_surface_resource);
}

void ShellSurface::set_transient(struct wl_client *client,
                      struct wl_resource *shell_surface_resource,
                      struct wl_resource *parent_shell_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(client);
    Q_UNUSED(flags);
    ShellSurface *shell_surface = static_cast<ShellSurface *>(shell_surface_resource->data);
    ShellSurface *parent_shell_surface = static_cast<ShellSurface *>(parent_shell_surface_resource->data);
    QPointF point = parent_shell_surface->m_surface->pos() + QPoint(x,y);
    shell_surface->m_surface->setPos(point);
}

void ShellSurface::set_fullscreen(struct wl_client *client,
                       struct wl_resource *shell_surface_resource)
{
    Q_UNUSED(client);
    Q_UNUSED(shell_surface_resource);
}

void ShellSurface::set_popup(wl_client *client, wl_resource *resource, wl_resource *input_device, uint32_t time, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    Q_UNUSED(parent);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(flags);
}

const struct wl_shell_surface_interface ShellSurface::shell_surface_interface = {
    ShellSurface::move,
    ShellSurface::resize,
    ShellSurface::set_toplevel,
    ShellSurface::set_transient,
    ShellSurface::set_fullscreen,
    ShellSurface::set_popup

};

}
