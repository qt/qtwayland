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

#ifndef WLEXTENDEDOUTPUT_H
#define WLEXTENDEDOUTPUT_H

#include "wayland-server.h"
#include "wayland-output-extension-server-protocol.h"

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/qnamespace.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Compositor;
class Output;

class OutputExtensionGlobal
{
public:
    OutputExtensionGlobal(Compositor *compositor);

private:
    Compositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
    static void get_extended_output(struct wl_client *client,
                                        struct wl_resource *output_extension_resource,
                                        uint32_t id,
                                        struct wl_resource *output_resource);
    static const struct wl_output_extension_interface output_extension_interface;

};

class ExtendedOutput
{
public:
    ExtendedOutput(struct wl_client *client, uint32_t id, Output *output, Compositor *compositor);

    Qt::ScreenOrientations orientationUpdateMask() { return m_orientationUpdateMask; }

    void sendOutputOrientation(Qt::ScreenOrientation orientation);

    static void destroy_resource(wl_resource *resource);

    static void set_orientation_update_mask(struct wl_client *client,
                                            struct wl_resource *resource,
                                            int32_t orientation_update_mask);

private:
    struct wl_resource *m_extended_output_resource;
    Output *m_output;
    Compositor *m_compositor;
    Qt::ScreenOrientations m_orientationUpdateMask;
};

}

QT_END_NAMESPACE

#endif // WLEXTENDEDOUTPUT_H
