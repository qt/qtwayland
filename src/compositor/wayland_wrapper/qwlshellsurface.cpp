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

#include "qwlshellsurface_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"
#include "qwlinputdevice_p.h"
#include "qwlsubsurface_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

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
    Surface *surface = resolve<Surface>(surface_super);
    new ShellSurface(client,id,surface);
}

const struct wl_shell_interface Shell::shell_interface = {
    Shell::get_shell_surface
};

ShellSurface::ShellSurface(wl_client *client, uint32_t id, Surface *surface)
    : m_surface(surface)
    , m_resizeGrabber(0)
    , m_moveGrabber(0)
    , m_transientParent(0)
    , m_xOffset(0)
    , m_yOffset(0)
{
    m_shellSurface = wl_client_add_object(client,&wl_shell_surface_interface,&shell_surface_interface,id,this);
    surface->setShellSurface(this);
}

void ShellSurface::sendConfigure(uint32_t edges, int32_t width, int32_t height)
{
    wl_shell_surface_send_configure(m_shellSurface,edges,width,height);
}

Surface *ShellSurface::surface() const
{
    return m_surface;
}

void ShellSurface::adjustPosInResize()
{
    if (m_transientParent)
        return;
    if (!m_resizeGrabber || !(m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP_LEFT))
        return;

    int bottomLeftX = wl_fixed_to_int(m_resizeGrabber->base()->x) + m_resizeGrabber->width;
    int bottomLeftY = wl_fixed_to_int(m_resizeGrabber->base()->y) + m_resizeGrabber->height;
    qreal x = surface()->pos().x();
    qreal y = surface()->pos().y();
    if (m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP)
        y = bottomLeftY - surface()->size().height();
    if (m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_LEFT)
        x = bottomLeftX - surface()->size().width();
    QPointF newPos(x,y);
    surface()->setPos(newPos);
}

QPointF ShellSurface::adjustedPosToTransientParent() const
{
    if (!m_transientParent ||
            (m_surface->subSurface() && m_surface->subSurface()->parent()))
        return m_surface->nonAdjustedPos();

    return m_transientParent->surface()->pos() + QPoint(m_xOffset,m_yOffset);
}

void ShellSurface::resetResizeGrabber()
{
    m_resizeGrabber = 0;
}

void ShellSurface::resetMoveGrabber()
{
    m_moveGrabber = 0;
}

ShellSurface *ShellSurface::transientParent() const
{
    return m_transientParent;
}

void ShellSurface::setOffset(const QPointF &offset)
{
    m_xOffset = offset.x();
    m_yOffset = offset.y();
}

void ShellSurface::move(struct wl_client *client,
                struct wl_resource *shell_surface_resource,
                struct wl_resource *input_device_super,
                uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(time);
    ShellSurface *self = static_cast<ShellSurface *>(shell_surface_resource->data);
    InputDevice *input_device = static_cast<InputDevice *>(input_device_super->data);
    if (self->m_resizeGrabber || self->m_moveGrabber) {
        qDebug() << "invalid state";
        return;
    }

    self->m_moveGrabber = new ShellSurfaceMoveGrabber(self);
    wl_pointer *pointer = input_device->pointerDevice();
    self->m_moveGrabber->base()->x = pointer->x;
    self->m_moveGrabber->base()->y = pointer->y;
    self->m_moveGrabber->offset_x = wl_fixed_to_int(pointer->x) - self->surface()->pos().x();
    self->m_moveGrabber->offset_y = wl_fixed_to_int(pointer->y) - self->surface()->pos().y();

    wl_pointer_start_grab(pointer, self->m_moveGrabber->base());
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
    ShellSurface *self = static_cast<ShellSurface *>(shell_surface_resource->data);
    InputDevice *input_device = static_cast<InputDevice *>(input_device_super->data);
    if (self->m_moveGrabber || self->m_resizeGrabber) {
        qDebug() << "invalid state2";
        return;
    }
    self->m_resizeGrabber = new ShellSurfaceResizeGrabber(self);
    wl_pointer *pointer = input_device->pointerDevice();
    self->m_resizeGrabber->base()->x = pointer->x;
    self->m_resizeGrabber->base()->y = pointer->y;
    self->m_resizeGrabber->resize_edges = wl_shell_surface_resize(edges);
    self->m_resizeGrabber->width = self->surface()->size().width();
    self->m_resizeGrabber->height = self->surface()->size().height();

    wl_pointer_start_grab(pointer, self->m_resizeGrabber->base());
}

void ShellSurface::set_toplevel(struct wl_client *client,
                     struct wl_resource *shell_surface_resource)
{
    Q_UNUSED(client);
    ShellSurface *self = static_cast<ShellSurface *>(shell_surface_resource->data);
    self->m_transientParent = 0;
    self->m_xOffset = 0;
    self->m_yOffset = 0;

}

void ShellSurface::set_transient(struct wl_client *client,
                      struct wl_resource *shell_surface_resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(client);
    Q_UNUSED(flags);
    ShellSurface *shell_surface = static_cast<ShellSurface *>(shell_surface_resource->data);
    Surface *parent_surface = static_cast<Surface *>(parent_surface_resource->data);
    shell_surface->m_transientParent = parent_surface->shellSurface();
    shell_surface->m_xOffset = x;
    shell_surface->m_yOffset = y;
    if (flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE)
        shell_surface->surface()->setTransientInactive(true);
}

void ShellSurface::set_fullscreen(struct wl_client *client,
                       struct wl_resource *shell_surface_resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output)
{
    Q_UNUSED(client);
    Q_UNUSED(shell_surface_resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    Q_UNUSED(output);
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

void ShellSurface::set_maximized(struct wl_client *client,
                       struct wl_resource *shell_surface_resource,
                       struct wl_resource *output)
{
    Q_UNUSED(client);
    Q_UNUSED(shell_surface_resource);
    Q_UNUSED(output);
}

void ShellSurface::pong(struct wl_client *client,
                        struct wl_resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(client);
    Q_UNUSED(resource);
    Q_UNUSED(serial);
}

void ShellSurface::set_title(struct wl_client *client,
                             struct wl_resource *resource,
                             const char *title)
{
    Q_UNUSED(client);
    ShellSurface *self = static_cast<ShellSurface *>(resource->data);
    self->surface()->setTitle(QString::fromUtf8(title));
}

void ShellSurface::set_class(struct wl_client *client,
                             struct wl_resource *resource,
                             const char *class_)
{
    Q_UNUSED(client);
    ShellSurface *self = static_cast<ShellSurface *>(resource->data);
    self->surface()->setClassName(QString::fromUtf8(class_));
}

const struct wl_shell_surface_interface ShellSurface::shell_surface_interface = {
    ShellSurface::pong,
    ShellSurface::move,
    ShellSurface::resize,
    ShellSurface::set_toplevel,
    ShellSurface::set_transient,
    ShellSurface::set_fullscreen,
    ShellSurface::set_popup,
    ShellSurface::set_maximized,
    ShellSurface::set_title,
    ShellSurface::set_class
};

Qt::MouseButton toQtButton(uint32_t button)
{
#ifndef BTN_LEFT
    static const uint32_t BTN_LEFT = 0x110;
    static const uint32_t BTN_RIGHT = 0x111;
    static const uint32_t BTN_MIDDLE = 0x112;
#endif
    switch (button) {
    case BTN_LEFT:
        return Qt::LeftButton;
    case BTN_RIGHT:
        return Qt::RightButton;
    case BTN_MIDDLE:
        return Qt::MiddleButton;
    default:
        return Qt::NoButton;
    }
}

ShellSurfaceGrabber::ShellSurfaceGrabber(ShellSurface *shellSurface, const struct wl_pointer_grab_interface *interface)
    : shell_surface(shellSurface)
{
    base()->interface = interface;
    base()->focus = shell_surface->surface()->base();
}

ShellSurfaceGrabber::~ShellSurfaceGrabber()
{
}


void ShellSurfaceGrabber::destroy(wl_listener *listener, wl_resource *resource, uint32_t time)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);
    Q_UNUSED(listener);
    //ShellSurfaceGrabber *shell_surface_grabber = container_of(listener, ShellSurfaceGrabber,surface_destroy_listener);
    Q_ASSERT(false); //hasn't been enabled yet
    //wl_input_device_end_grab(shell_surface_grabber->base()->input_device,Compositor::currentTimeMsecs());
}


ShellSurfaceResizeGrabber::ShellSurfaceResizeGrabber(ShellSurface *shellSurface)
    : ShellSurfaceGrabber(shellSurface,&resize_grabber_interface)
{
}

void ShellSurfaceResizeGrabber::focus(wl_pointer_grab *grab, wl_surface *surface, int32_t x, int32_t y)
{
    Q_UNUSED(grab);
    Q_UNUSED(surface);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void ShellSurfaceResizeGrabber::motion(wl_pointer_grab *grab, uint32_t time, int32_t x, int32_t y)
{
    Q_UNUSED(time);
    Q_UNUSED(x);
    Q_UNUSED(y);
    //Should be more structured
    ShellSurfaceResizeGrabber *resize_grabber = reinterpret_cast<ShellSurfaceResizeGrabber *>(grab);
    ShellSurface *shell_surface = resize_grabber->shell_surface;
    wl_pointer *pointer = grab->pointer;
    int width_delta = wl_fixed_to_int(grab->x) - wl_fixed_to_int(pointer->x);
    int height_delta = wl_fixed_to_int(grab->y) - wl_fixed_to_int(pointer->y);
    int new_width = resize_grabber->width;
    int new_height = resize_grabber->height;
    if (resize_grabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP_LEFT) {
        if (resize_grabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP) {
            if (new_height + height_delta > 0) {
                new_height += height_delta;
            } else {
                new_height = 1;
            }
        }
        if (resize_grabber->resize_edges & WL_SHELL_SURFACE_RESIZE_LEFT) {
            if (new_width + width_delta > 0) {
                new_width += width_delta;
            } else {
                new_width = 1;
            }
        }
    }

    if (resize_grabber->resize_edges & WL_SHELL_SURFACE_RESIZE_BOTTOM) {
        if (new_height - height_delta > 0) {
            new_height -= height_delta;
        } else {
            new_height = 1;
        }
    }
    if (resize_grabber->resize_edges & WL_SHELL_SURFACE_RESIZE_RIGHT) {
        if (new_width - width_delta > 0) {
            new_width -= width_delta;
        } else {
            new_width =1;
        }
    }

    shell_surface->sendConfigure(resize_grabber->resize_edges,new_width,new_height);
}

void ShellSurfaceResizeGrabber::button(wl_pointer_grab *grab, uint32_t time, uint32_t button, uint32_t state)
{
    Q_UNUSED(time)
    ShellSurfaceResizeGrabber *self = reinterpret_cast<ShellSurfaceResizeGrabber *>(grab);
    ShellSurface *shell_surface = self->shell_surface;
    if (toQtButton(button) == Qt::LeftButton && !state) {
        wl_pointer_end_grab(grab->pointer);
        shell_surface->resetResizeGrabber();
        delete self;
    }
}

const struct wl_pointer_grab_interface ShellSurfaceResizeGrabber::resize_grabber_interface = {
    ShellSurfaceResizeGrabber::focus,
    ShellSurfaceResizeGrabber::motion,
    ShellSurfaceResizeGrabber::button
};

ShellSurfaceMoveGrabber::ShellSurfaceMoveGrabber(ShellSurface *shellSurface)
    : ShellSurfaceGrabber(shellSurface,&move_grabber_interface)
{
}

void ShellSurfaceMoveGrabber::focus(wl_pointer_grab *grab, wl_surface *surface, int32_t x, int32_t y)
{
    Q_UNUSED(grab);
    Q_UNUSED(surface);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void ShellSurfaceMoveGrabber::motion(wl_pointer_grab *grab, uint32_t time, int32_t x, int32_t y)
{
    Q_UNUSED(time);
    Q_UNUSED(x);
    Q_UNUSED(y);
    ShellSurfaceMoveGrabber *shell_surface_grabber = reinterpret_cast<ShellSurfaceMoveGrabber *>(grab);
    ShellSurface *shell_surface = shell_surface_grabber->shell_surface;
    wl_pointer *pointer = grab->pointer;
    QPointF pos(wl_fixed_to_int(pointer->x) - shell_surface_grabber->offset_x,
                wl_fixed_to_int(pointer->y) - shell_surface_grabber->offset_y);
    shell_surface->surface()->setPos(pos);
    if (shell_surface->transientParent())
        shell_surface->setOffset(pos - shell_surface->transientParent()->surface()->pos());
    shell_surface->surface()->damage(QRect(QPoint(0,0),shell_surface->surface()->size()));
}

void ShellSurfaceMoveGrabber::button(wl_pointer_grab *grab, uint32_t time, uint32_t button, uint32_t state)
{
    Q_UNUSED(time)
    ShellSurfaceResizeGrabber *self = reinterpret_cast<ShellSurfaceResizeGrabber *>(grab);
    ShellSurface *shell_surface = self->shell_surface;
    if (toQtButton(button) == Qt::LeftButton && !state) {
        wl_pointer_set_focus(grab->pointer, 0, 0, 0);
        wl_pointer_end_grab(grab->pointer);
        shell_surface->resetMoveGrabber();
        delete self;
    }
}

const struct wl_pointer_grab_interface ShellSurfaceMoveGrabber::move_grabber_interface = {
    ShellSurfaceMoveGrabber::focus,
    ShellSurfaceMoveGrabber::motion,
    ShellSurfaceMoveGrabber::button
};

}

QT_END_NAMESPACE
