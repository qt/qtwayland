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
#include "qwlpointer_p.h"
#include "qwlextendedsurface_p.h"

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

ShellSurfacePopupGrabber *Shell::getPopupGrabber(InputDevice *input)
{
    if (!m_popupGrabber.contains(input))
        m_popupGrabber.insert(input, new ShellSurfacePopupGrabber(input));

    return m_popupGrabber.value(input);
}

void Shell::get_shell_surface(struct wl_client *client,
              struct wl_resource *shell_resource,
              uint32_t id,
              struct wl_resource *surface_super)
{
    Shell *shell = static_cast<Shell*>(shell_resource->data);
    Surface *surface = Surface::fromResource(surface_super);
    new ShellSurface(shell, client, id, surface);
}

const struct wl_shell_interface Shell::shell_interface = {
    Shell::get_shell_surface
};

ShellSurface::ShellSurface(Shell *shell, wl_client *client, uint32_t id, Surface *surface)
    : wl_shell_surface(client, id)
    , m_shell(shell)
    , m_surface(surface)
    , m_resizeGrabber(0)
    , m_moveGrabber(0)
    , m_popupGrabber(0)
    , m_transientParent(0)
    , m_xOffset(0)
    , m_yOffset(0)
    , m_windowType(QWaylandSurface::None)
    , m_popupLocation()
    , m_popupSerial()
{
    surface->setShellSurface(this);
}

void ShellSurface::sendConfigure(uint32_t edges, int32_t width, int32_t height)
{
    send_configure(edges, width, height);
}

void ShellSurface::ping()
{
    uint32_t serial = wl_display_next_serial(m_surface->compositor()->wl_display());
    m_pings.insert(serial);
    send_ping(serial);
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

    int bottomLeftX = m_resizeGrabber->point.x() + m_resizeGrabber->width;
    int bottomLeftY = m_resizeGrabber->point.y() + m_resizeGrabber->height;
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

QWaylandSurface::WindowType ShellSurface::windowType() const
{
    return m_windowType;
}

void ShellSurface::mapPopup()
{
    if (m_popupGrabber->grabSerial() == m_popupSerial) {
        m_popupGrabber->addPopup(this);
    } else {
        send_popup_done();
        m_popupGrabber->setClient(0);
    }
}

void ShellSurface::shell_surface_destroy_resource(Resource *)
{
    if (m_popupGrabber)
        m_popupGrabber->removePopup(this);

    delete this;
}

void ShellSurface::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t time)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);

    if (m_resizeGrabber || m_moveGrabber) {
        qDebug() << "invalid state";
        return;
    }

    InputDevice *input_device = InputDevice::fromSeatResource(input_device_super);
    Pointer *pointer = input_device->pointerDevice();

    m_moveGrabber = new ShellSurfaceMoveGrabber(this, pointer->position() - surface()->pos());

    pointer->startGrab(m_moveGrabber);
}

void ShellSurface::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t time,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);
    Q_UNUSED(edges);

    if (m_moveGrabber || m_resizeGrabber) {
        qDebug() << "invalid state2";
        return;
    }

    m_resizeGrabber = new ShellSurfaceResizeGrabber(this);

    InputDevice *input_device = InputDevice::fromSeatResource(input_device_super);
    Pointer *pointer = input_device->pointerDevice();

    m_resizeGrabber->point = pointer->position();
    m_resizeGrabber->resize_edges = static_cast<wl_shell_surface_resize>(edges);
    m_resizeGrabber->width = surface()->size().width();
    m_resizeGrabber->height = surface()->size().height();

    pointer->startGrab(m_resizeGrabber);
}

void ShellSurface::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    m_transientParent = 0;
    m_xOffset = 0;
    m_yOffset = 0;

    if (m_windowType != QWaylandSurface::Toplevel) {
        m_windowType = QWaylandSurface::Toplevel;
        emit m_surface->waylandSurface()->windowTypeChanged(m_windowType);
    }

    if (m_surface->extendedSurface())
        m_surface->extendedSurface()->setVisibility(QWindow::Windowed, false);
}

void ShellSurface::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_UNUSED(flags);
    Surface *parent_surface = Surface::fromResource(parent_surface_resource);
    m_transientParent = parent_surface->shellSurface();
    m_xOffset = x;
    m_yOffset = y;
    if (flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE)
        surface()->setTransientInactive(true);

    if (m_windowType != QWaylandSurface::Transient) {
        m_windowType = QWaylandSurface::Transient;
        emit m_surface->waylandSurface()->windowTypeChanged(m_windowType);
    }

    if (m_surface->extendedSurface())
        m_surface->extendedSurface()->setVisibility(QWindow::AutomaticVisibility, false);
}

void ShellSurface::shell_surface_set_fullscreen(Resource *resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    Q_UNUSED(output);
    QSize defaultScreenSize = m_surface->compositor()->outputGeometry().size();
    send_configure(resize_bottom_right, defaultScreenSize.width(), defaultScreenSize.height());

    if (m_surface->extendedSurface())
        m_surface->extendedSurface()->setVisibility(QWindow::FullScreen, false);
}

void ShellSurface::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(input_device);
    Q_UNUSED(flags);

    InputDevice *input = InputDevice::fromSeatResource(input_device);
    m_popupGrabber = m_shell->getPopupGrabber(input);

    m_popupSerial = serial;
    m_transientParent = Surface::fromResource(parent)->shellSurface();
    m_popupLocation = QPointF(x, y);

    if (m_windowType != QWaylandSurface::Popup) {
        m_windowType = QWaylandSurface::Popup;
        emit m_surface->waylandSurface()->windowTypeChanged(m_windowType);
    }

    if (m_surface->extendedSurface())
        m_surface->extendedSurface()->setVisibility(QWindow::AutomaticVisibility, false);
}

void ShellSurface::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output)
{
    Q_UNUSED(resource);
    Q_UNUSED(output);
    QSize defaultScreenSize = m_surface->compositor()->outputGeometry().size();
    send_configure(resize_bottom_right, defaultScreenSize.width(), defaultScreenSize.height());

    if (m_surface->extendedSurface())
        m_surface->extendedSurface()->setVisibility(QWindow::Maximized, false);
}

void ShellSurface::shell_surface_pong(Resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(resource);
    if (m_pings.remove(serial))
        emit m_surface->waylandSurface()->pong();
    else
        qWarning("Received an unexpected pong!");
}

void ShellSurface::shell_surface_set_title(Resource *resource,
                             const QString &title)
{
    Q_UNUSED(resource);
    surface()->setTitle(title);
}

void ShellSurface::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    surface()->setClassName(className);
}

ShellSurfaceGrabber::ShellSurfaceGrabber(ShellSurface *shellSurface)
    : PointerGrabber()
    , shell_surface(shellSurface)
{
}

ShellSurfaceGrabber::~ShellSurfaceGrabber()
{
}

ShellSurfaceResizeGrabber::ShellSurfaceResizeGrabber(ShellSurface *shellSurface)
    : ShellSurfaceGrabber(shellSurface)
{
}

void ShellSurfaceResizeGrabber::focus()
{
}

void ShellSurfaceResizeGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    int width_delta = point.x() - m_pointer->position().x();
    int height_delta = point.y() - m_pointer->position().y();

    int new_height = height;
    if (resize_edges & WL_SHELL_SURFACE_RESIZE_TOP)
        new_height = qMax(new_height + height_delta, 1);
    else if (resize_edges & WL_SHELL_SURFACE_RESIZE_BOTTOM)
        new_height = qMax(new_height - height_delta, 1);

    int new_width = width;
    if (resize_edges & WL_SHELL_SURFACE_RESIZE_LEFT)
        new_width = qMax(new_width + width_delta, 1);
    else if (resize_edges & WL_SHELL_SURFACE_RESIZE_RIGHT)
        new_width = qMax(new_width - width_delta, 1);

    shell_surface->sendConfigure(resize_edges, new_width, new_height);
}

void ShellSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        m_pointer->endGrab();
        shell_surface->resetResizeGrabber();
        delete this;
    }
}

ShellSurfaceMoveGrabber::ShellSurfaceMoveGrabber(ShellSurface *shellSurface, const QPointF &offset)
    : ShellSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
}

void ShellSurfaceMoveGrabber::focus()
{
}

void ShellSurfaceMoveGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    QPointF pos(m_pointer->position() - m_offset);
    shell_surface->surface()->setPos(pos);
    if (shell_surface->transientParent())
        shell_surface->setOffset(pos - shell_surface->transientParent()->surface()->pos());

}

void ShellSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        m_pointer->setFocus(0, QPointF());
        m_pointer->endGrab();
        shell_surface->resetMoveGrabber();
        delete this;
    }
}

ShellSurfacePopupGrabber::ShellSurfacePopupGrabber(InputDevice *inputDevice)
    : PointerGrabber()
    , m_inputDevice(inputDevice)
    , m_client(0)
    , m_surfaces()
    , m_initialUp(false)
{
}

uint32_t ShellSurfacePopupGrabber::grabSerial() const
{
    return m_inputDevice->pointerDevice()->grabSerial();
}

struct ::wl_client *ShellSurfacePopupGrabber::client() const
{
    return m_client;
}

void ShellSurfacePopupGrabber::setClient(struct ::wl_client *client)
{
    m_client = client;
}

void ShellSurfacePopupGrabber::addPopup(ShellSurface *surface)
{
    if (m_surfaces.isEmpty()) {
        m_client = surface->resource()->client();

        if (m_inputDevice->pointerDevice()->buttonPressed())
            m_initialUp = false;

        m_surfaces.append(surface);
        m_inputDevice->pointerDevice()->startGrab(this);
    } else {
        m_surfaces.append(surface);
    }
}

void ShellSurfacePopupGrabber::removePopup(ShellSurface *surface)
{
    if (m_surfaces.isEmpty())
        return;

    m_surfaces.removeOne(surface);
    if (m_surfaces.isEmpty())
        m_inputDevice->pointerDevice()->endGrab();
}

void ShellSurfacePopupGrabber::focus()
{
    if (m_pointer->current() && m_pointer->current()->resource()->client() == m_client)
        m_pointer->setFocus(m_pointer->current(), m_pointer->currentPosition());
    else
        m_pointer->setFocus(0, QPointF());
}

void ShellSurfacePopupGrabber::motion(uint32_t time)
{
    m_pointer->motion(time);
}

void ShellSurfacePopupGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    if (m_pointer->focusResource()) {
        m_pointer->sendButton(time, button, state);
    } else if (state == QtWaylandServer::wl_pointer::button_state_pressed &&
               (m_initialUp || time - m_pointer->grabTime() > 500) &&
               m_pointer->currentGrab() == this) {
        m_pointer->endGrab();
        Q_FOREACH (ShellSurface *surface, m_surfaces) {
            surface->send_popup_done();
        }
        m_surfaces.clear();
    }

    if (state == QtWaylandServer::wl_pointer::button_state_released)
        m_initialUp = true;
}


}

QT_END_NAMESPACE
