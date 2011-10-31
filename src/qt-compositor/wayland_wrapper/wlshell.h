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

#ifndef WLSHELL_H
#define WLSHELL_H

#include <waylandobject.h>

namespace Wayland {

class Shell : public Object<struct wl_object>
{
public:
    Shell();

    static void bind_func(struct wl_client *client, void *data,
                                uint32_t version, uint32_t id);
    static void shell_move(struct wl_client *client,
                    struct wl_resource *shell,
                    struct wl_resource *surface,
                    struct wl_resource *input_device,
                    uint32_t time);
    static void shell_resize(struct wl_client *client,
                      struct wl_resource *shell,
                      struct wl_resource *surface,
                      struct wl_resource *input_device,
                      uint32_t time,
                      uint32_t edges);
    static void shell_drag(struct wl_client *client,
                    struct wl_resource *shell,
                    uint32_t id);
    static void shell_selection(struct wl_client *client,
                         struct wl_resource *shell,
                         uint32_t id);
    static void set_toplevel(struct wl_client *client,
                         struct wl_resource *shell,
                         struct wl_resource *surface);
    static void set_transient(struct wl_client *client,
                          struct wl_resource *shell,
                          struct wl_resource *surface,
                          struct wl_resource *parent,
                          int x,
                          int y,
                          uint32_t flags);
    static void set_fullscreen(struct wl_client *client,
                           struct wl_resource *shell,
                           struct wl_resource *surface);
    const static struct wl_shell_interface shell_interface;
};

}

#endif // WLSHELL_H
