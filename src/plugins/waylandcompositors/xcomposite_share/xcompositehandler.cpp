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

#include "xcompositehandler.h"

#include "wayland-xcomposite-server-protocol.h"

#include "xcompositebuffer.h"
#include <X11/extensions/Xcomposite.h>

QT_USE_NAMESPACE

XCompositeHandler::XCompositeHandler(QtWayland::Compositor *compositor, Display *display, QWindow *window)
    : mCompositor(compositor)
    , mwindow(window)
    , mDisplay(display)
{
    mCompositor->window()->create();

    mFakeRootWindow = new QWindow(mCompositor->window());
    mFakeRootWindow->setGeometry(QRect(-1,-1,1,1));
    mFakeRootWindow->create();
    mFakeRootWindow->show();
    int composite_event_base, composite_error_base;
    if (XCompositeQueryExtension(mDisplay, &composite_event_base, &composite_error_base)) {

    } else {
        qFatal("XComposite required");
    }
}

void XCompositeHandler::createBuffer(struct wl_client *client, uint32_t id, Window window, const QSize &size)
{
    XCompositeBuffer *buffer = new XCompositeBuffer(mCompositor, window, size);
    buffer->addClientResource(client, &buffer->base()->resource,
                              id,&wl_buffer_interface,
                              &XCompositeBuffer::buffer_interface,
                              XCompositeBuffer::delete_resource);
}

void XCompositeHandler::xcomposite_bind_func(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    XCompositeHandler *handler = static_cast<XCompositeHandler *>(data);
    wl_resource *resource = wl_client_add_object(client,&wl_xcomposite_interface,&xcomposite_interface,id,handler);
    const char *displayString = XDisplayString(handler->mDisplay);
    wl_resource_post_event(resource, WL_XCOMPOSITE_ROOT, displayString, handler->mFakeRootWindow->winId());
}

void XCompositeHandler::create_buffer(struct wl_client *client,
                      struct wl_resource *xcomposite,
                      uint32_t id,
                      uint32_t x_window,
                      int32_t width,
                      int32_t height)
{
    Window window = (Window)x_window;
    XCompositeHandler *that = reinterpret_cast<XCompositeHandler *>(xcomposite);
    that->createBuffer(client, id, window, QSize(width,height));
}
