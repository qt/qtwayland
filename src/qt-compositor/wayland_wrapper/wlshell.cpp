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

#include "wlshell.h"

#include "wlcompositor.h"

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

void Shell::shell_move(struct wl_client *client,
                struct wl_resource *shell,
                struct wl_resource *surface,
                struct wl_resource *input_device,
                uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    qDebug() << "shellMove";
}

void Shell::shell_resize(struct wl_client *client,
                  struct wl_resource *shell,
                  struct wl_resource *surface,
                  struct wl_resource *input_device,
                  uint32_t time,
                  uint32_t edges)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    Q_UNUSED(edges);
    qDebug() << "shellResize";
}

void Shell::shell_drag(struct wl_client *client,
                struct wl_resource *shell,
                uint32_t id)
{
    Q_UNUSED(shell);
    qDebug() << "shellDrag";
    Drag::instance()->create(client, id);
}

void Shell::shell_selection(struct wl_client *client,
                     struct wl_resource  *shell,
                     uint32_t id)
{
    qDebug() << "shellSelection";
    Q_UNUSED(shell);
    Selection::instance()->create(client, id);
}

void Shell::set_toplevel(struct wl_client *client,
                     struct wl_resource *wl_shell,
                     struct wl_resource  *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(wl_shell);
    Q_UNUSED(surface);
}

void Shell::set_transient(struct wl_client *client,
                      struct wl_resource *wl_shell,
                      struct wl_resource *surface,
                      struct wl_resource *parent,
                      int x,
                      int y,
                      uint32_t flags)
{
    Q_UNUSED(client);
    Q_UNUSED(wl_shell);
    Q_UNUSED(surface);
    Q_UNUSED(parent);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(flags);
}
void Shell::set_fullscreen(struct wl_client *client,
                       struct wl_resource *wl_shell,
                       struct wl_resource *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(wl_shell);
    Q_UNUSED(surface);
}

const struct wl_shell_interface Shell::shell_interface = {
    shell_move,
    shell_resize,
    shell_drag,
    shell_selection,
    set_toplevel,
    set_transient,
    set_fullscreen
};

}
