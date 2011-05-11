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

#include "wlcompositor.h"

#include "waylandobject.h"
#include "wldisplay.h"
#include "wlshmbuffer.h"
#include "wlsurface.h"
#include "wlselection.h"
#include "waylandcompositor.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QSocketNotifier>
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>

#include <wayland-server.h>

#include "hardware_integration/graphicshardwareintegration.h"

namespace Wayland {

void input_device_attach(struct wl_client *client,
                         struct wl_input_device *device_base,
                         uint32_t time,
                         struct wl_buffer *buffer, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    Q_UNUSED(device_base);
    Q_UNUSED(time);
    Q_UNUSED(buffer);
    Q_UNUSED(x);
    Q_UNUSED(y);

    qDebug() << "Client %p input device attach" << client;
}

const static struct wl_input_device_interface input_device_interface = {
    input_device_attach,
};

void destroy_surface(struct wl_resource *resource, struct wl_client *client)
{
    Q_UNUSED(client);
    Surface *surface = wayland_cast<Surface *>((wl_surface *)resource);
    delete surface;
}

void compositor_create_surface(struct wl_client *client,
			       struct wl_compositor *compositor, uint32_t id)
{
    wayland_cast<Compositor *>(compositor)->createSurface(client, id);
}

const static struct wl_compositor_interface compositor_interface = {
    compositor_create_surface
};

void shell_move(struct wl_client *client,
                struct wl_shell *shell,
                struct wl_surface *surface,
                struct wl_input_device *input_device,
                uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    qDebug() << "shellMove";
}

void shell_resize(struct wl_client *client,
                  struct wl_shell *shell,
                  struct wl_surface *surface,
                  struct wl_input_device *input_device,
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

void shell_drag(struct wl_client *client,
                struct wl_shell *shell,
                uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(id);
    qDebug() << "shellDrag";
}

void shell_selection(struct wl_client *client,
                     struct wl_shell *shell,
                     uint32_t id)
{
    qDebug() << "shellSelection";
    Q_UNUSED(shell);
    Selection::instance()->create(client, id);
}

const static struct wl_shell_interface shell_interface = {
    shell_move,
    shell_resize,
    shell_drag,
    shell_selection
};

static Compositor *compositor;

Compositor *Compositor::instance()
{
    return compositor;
}

Compositor::Compositor(WaylandCompositor *qt_compositor)
    : m_display(new Display)
    , m_shm(m_display)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_pointerFocusSurface(0)
    , m_keyFocusSurface(0)
    , m_directRenderSurface(0)
{
    compositor = this;

#if defined (QT_COMPOSITOR_WAYLAND_GL)
    m_graphics_hw_integration = GraphicsHardwareIntegration::createGraphicsHardwareIntegration(qt_compositor);
#endif

    if (wl_compositor_init(base(), &compositor_interface, m_display->handle())) {
        fprintf(stderr, "Fatal: Error initializing compositor\n");
        exit(EXIT_FAILURE);
    }

    memset(&m_input, 0, sizeof(m_input));

    m_display->addGlobalObject(m_output.base(), &wl_output_interface, 0, output_post_geometry);
    m_display->addGlobalObject(&m_shell, &wl_shell_interface, &shell_interface, 0);

    wl_input_device_init(&m_input, base());
    m_display->addGlobalObject(&m_input.object, &wl_input_device_interface, &input_device_interface, 0);

    if (wl_display_add_socket(m_display->handle(), 0)) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));
}

Compositor::~Compositor()
{
    delete m_display;
}

void Compositor::frameFinished()
{
    QSet<Surface *>::iterator i;
    for (i = m_dirty_surfaces.begin(); i != m_dirty_surfaces.end(); i++) {
        wl_display_post_frame(m_display->handle(), (*i)->base(), currentTimeMsecs());
    }
    m_dirty_surfaces.clear();
}

void Compositor::createSurface(struct wl_client *client, int id)
{
    Surface *surface = new Surface(client, this);
    printf("Compositor::createSurface: %p %d\n", client, id);

    addClientResource(client, &surface->base()->resource, id, &wl_surface_interface,
            &surface_interface, destroy_surface);

    m_qt_compositor->surfaceCreated(surface->handle());

    QList<struct wl_client *> prevClientList = clients();
    m_surfaces << surface;
    if (!prevClientList.contains(client))
        emit clientAdded(client);
}

struct wl_client *Compositor::getClientFromWinId(uint winId) const
{
    Surface *surface = getSurfaceFromWinId(winId);
    if (surface)
        return surface->base()->client;

    return 0;
}

Surface *Compositor::getSurfaceFromWinId(uint winId) const
{
    foreach (Surface *surface, m_surfaces) {
        if (surface->id() == winId)
            return surface;
    }

    return 0;
}

QImage Compositor::image(uint winId) const
{
    printf("Compositor::image(%x)\n", winId);
    foreach (Surface *surface, m_surfaces) {
        if (surface->id() == winId) {
            return surface->image();
        }
    }

    return QImage();
}

uint Compositor::currentTimeMsecs()
{
    //### we throw away the time information
    struct timeval tv;
    int ret = gettimeofday(&tv, 0);
    if (ret == 0)
        return tv.tv_sec*1000 + tv.tv_usec/1000;
    return 0;
}

void Compositor::processWaylandEvents()
{
    int ret = wl_event_loop_dispatch(m_loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
}

void Compositor::surfaceDestroyed(Surface *surface)
{
    m_surfaces.removeOne(surface);
    if (m_keyFocusSurface == surface)
        setKeyFocus(0);
    if (m_pointerFocusSurface == surface)
        setPointerFocus(0);
}

void Compositor::markSurfaceAsDirty(Wayland::Surface *surface)
{
    m_dirty_surfaces.insert(surface);
}

void Compositor::destroyClientForSurface(Surface *surface)
{
    wl_client *client = surface->base()->client;

    if (client)
        wl_client_destroy(client);
}

void Compositor::setInputFocus(Surface *surface)
{
    wl_surface *base = surface ? surface->base() : 0;

    ulong time = currentTimeMsecs();

    m_keyFocusSurface = surface;
    m_pointerFocusSurface = surface;
    wl_input_device_set_keyboard_focus(&m_input, base, time);
    wl_input_device_set_pointer_focus(&m_input, base, time, 0, 0, 0, 0);
}

void Compositor::setKeyFocus(Surface *surface)
{
    m_keyFocusSurface = surface;
    wl_input_device_set_keyboard_focus(&m_input, surface ? surface->base() : 0, currentTimeMsecs());
}

void Compositor::setPointerFocus(Surface *surface, const QPoint &pos)
{
    m_pointerFocusSurface = surface;
    wl_input_device_set_pointer_focus(&m_input, surface ? surface->base() : 0, currentTimeMsecs(), pos.x(), pos.y(), pos.x(), pos.y());
}

QWidget * Compositor::topLevelWidget() const
{
    return m_qt_compositor->topLevelWidget();
}

GraphicsHardwareIntegration * Compositor::graphicsHWIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_graphics_hw_integration;
#else
    return 0;
#endif
}

void Compositor::initializeHardwareIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    m_graphics_hw_integration->initializeHardware(m_display);
#endif
}

bool Compositor::setDirectRenderSurface(Surface *surface)
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_graphics_hw_integration->setDirectRenderSurface(surface ? surface->handle() : 0)) {
        m_directRenderSurface = surface;
        return true;
    }
#endif
    return false;
}

QList<struct wl_client *> Compositor::clients() const
{
    QList<struct wl_client *> list;
    foreach (Surface *surface, m_surfaces) {
        struct wl_client *client = surface->base()->client;
        if (!list.contains(client))
            list.append(client);
    }
    return list;
}

}

wl_input_device * Wayland::Compositor::defaultInputDevice()
{
    return &m_input;
}
