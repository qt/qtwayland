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

#include "wldisplay.h"
#include "wlshmbuffer.h"
#include "wlsurface.h"
#include "wlinputdevice.h"
#include "waylandcompositor.h"
#include "wldatadevicemanager.h"
#include "wldatadevice.h"
#include "wlextendedoutput.h"
#include "wlextendedsurface.h"
#include "wlsubsurface.h"

#include <QWindow>
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
#include "waylandwindowmanagerintegration.h"

namespace Wayland {

static Compositor *compositor;

void destroy_surface(struct wl_resource *resource)
{
    Surface *surface = wayland_cast<Surface *>((wl_surface *)resource);
    delete surface;
}

void compositor_create_surface(struct wl_client *client,
                               struct wl_resource *resource, uint32_t id)
{
     static_cast<Compositor *>(resource->data)->createSurface(client,id);
}

const static struct wl_compositor_interface compositor_interface = {
    compositor_create_surface
};

void Compositor::bind_func(struct wl_client *client, void *data,
                      uint32_t version, uint32_t id)
{
    wl_client_add_object(client,&wl_compositor_interface, &compositor_interface, id,data);
}


Compositor *Compositor::instance()
{
    return compositor;
}

Compositor::Compositor(WaylandCompositor *qt_compositor)
    : m_display(new Display)
    , m_shell(this)
    , m_shm(m_display)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_orientation(Qt::UnknownOrientation)
    , m_pointerFocusSurface(0)
    , m_keyFocusSurface(0)
    , m_directRenderSurface(0)
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , m_graphics_hw_integration(0)
#endif
    , m_retainNotify(0)
    , m_outputExtension(0)
    , m_surfaceExtension(0)
    , m_subSurfaceExtension(0)
{
    compositor = this;
    qDebug() << "Compositor instance is" << this;

#if defined (QT_COMPOSITOR_WAYLAND_GL)
    QWindow *window = qt_compositor->window();
    if (window && window->surfaceType() != QWindow::RasterSurface)
        m_graphics_hw_integration = GraphicsHardwareIntegration::createGraphicsHardwareIntegration(qt_compositor);
#endif
    m_windowManagerIntegration = new WindowManagerServerIntegration(this);

    wl_display_add_global(m_display->handle(),&wl_compositor_interface,this,Compositor::bind_func);

    m_data_device_manager =  new DataDeviceManager(this);

    m_input = new InputDevice(this);

    wl_display_add_global(m_display->handle(),&wl_output_interface, &m_output_global,OutputGlobal::output_bind_func);

    wl_display_add_global(m_display->handle(), &wl_shell_interface, &m_shell, Shell::bind_func);

    m_outputExtension = new OutputExtensionGlobal(this);
    m_surfaceExtension = new SurfaceExtensionGlobal(this);

    if (wl_display_add_socket(m_display->handle(), qt_compositor->socketName())) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));

    //initialize distancefieldglyphcache here
}

Compositor::~Compositor()
{
    delete m_input;
    delete m_data_device_manager;

    delete m_display;
}

void Compositor::frameFinished(Surface *surface)
{
    if (surface && m_dirty_surfaces.contains(surface)) {
        surface->sendFrameCallback();
	m_dirty_surfaces.remove(surface);
    } else if (!surface) {
	foreach (Surface *surface, m_dirty_surfaces)
            surface->sendFrameCallback();
	m_dirty_surfaces.clear();
    }
}

void Compositor::createSurface(struct wl_client *client, int id)
{
    Surface *surface = new Surface(client, this);
    printf("Compositor::createSurface: %p %d\n", client, id);

    addClientResource(client, &surface->base()->resource, id, &wl_surface_interface,
            &Surface::surface_interface, destroy_surface);

    QList<struct wl_client *> prevClientList = clients();
    m_surfaces << surface;

    //this is not how we want to solve this.
    if (!prevClientList.contains(client)) {
        emit clientAdded(client);
    }

    m_qt_compositor->surfaceCreated(surface->handle());
}

struct wl_client *Compositor::getClientFromWinId(uint winId) const
{
    Surface *surface = getSurfaceFromWinId(winId);
    if (surface)
        return surface->base()->resource.client;

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

void Compositor::releaseBuffer(void *bufferHandle)
{
    struct wl_buffer *buffer = static_cast<struct wl_buffer*>(bufferHandle);
    if (buffer) {
        //qDebug() << "WL_BUFFER_RELEASE" << buffer<< buffer->resource.client;
        wl_resource_post_event(&buffer->resource, WL_BUFFER_RELEASE);
    }

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
    m_dirty_surfaces.remove(surface);
    if (m_keyFocusSurface == surface)
        setKeyFocus(0);
    if (m_pointerFocusSurface == surface)
        setPointerFocus(0);
    if (m_directRenderSurface == surface)
        setDirectRenderSurface(0);
}

void Compositor::markSurfaceAsDirty(Wayland::Surface *surface)
{
    m_dirty_surfaces.insert(surface);
}

void Compositor::destroyClientForSurface(Surface *surface)
{
    wl_client *client = surface->base()->resource.client;

    if (client) {
        m_windowManagerIntegration->removeClient(client);
        wl_client_destroy(client);
    }
}

void Compositor::setInputFocus(Surface *surface)
{
    setKeyFocus(surface);
    setPointerFocus(surface);
}

void Compositor::setKeyFocus(Surface *surface)
{
    m_input->sendSelectionFocus(surface);
    m_keyFocusSurface = surface;
    wl_input_device_set_keyboard_focus(m_input->base(), surface ? surface->base() : 0, currentTimeMsecs());
}

Surface *Compositor::keyFocus() const
{
    return m_keyFocusSurface;
}

void Compositor::setPointerFocus(Surface *surface, const QPoint &pos)
{
    m_pointerFocusSurface = surface;
    wl_input_device_set_pointer_focus(m_input->base(), surface ? surface->base() : 0, currentTimeMsecs(), pos.x(), pos.y(), pos.x(), pos.y());
}

Surface *Compositor::pointerFocus() const
{
    return m_pointerFocusSurface;
}

QWindow *Compositor::window() const
{
    return m_qt_compositor->window();
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
    if (m_graphics_hw_integration)
        m_graphics_hw_integration->initializeHardware(m_display);
#endif
}

void Compositor::initializeWindowManagerProtocol()
{
    m_windowManagerIntegration->initialize(m_display);
}

void Compositor::enableSubSurfaceExtension()
{
    if (!m_subSurfaceExtension) {
        m_subSurfaceExtension = new SubSurfaceExtensionGlobal(this);
    }
}

bool Compositor::setDirectRenderSurface(Surface *surface)
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_graphics_hw_integration && m_graphics_hw_integration->setDirectRenderSurface(surface ? surface->handle() : 0)) {
        m_directRenderSurface = surface;
        return true;
    }
#else
    Q_UNUSED(surface);
#endif
    return false;
}

QList<struct wl_client *> Compositor::clients() const
{
    QList<struct wl_client *> list;
    foreach (Surface *surface, m_surfaces) {
        struct wl_client *client = surface->base()->resource.client;
        if (!list.contains(client))
            list.append(client);
    }
    return list;
}

void Compositor::setScreenOrientation(Qt::ScreenOrientation orientation)
{
    m_orientation = orientation;

    QList<struct wl_client*> clientList = clients();
    for (int i = 0; i < clientList.length(); ++i) {
        struct wl_client *client = clientList.at(i);
        Output *output = m_output_global.outputForClient(client);
        Q_ASSERT(output);
        if (output->extendedOutput()){
            output->extendedOutput()->sendOutputOrientation(orientation);
        }
    }
}

Qt::ScreenOrientation Compositor::screenOrientation() const
{
    return m_orientation;
}

void Compositor::setOutputGeometry(const QRect &geometry)
{
    m_output_global.setGeometry(geometry);
}

InputDevice* Compositor::defaultInputDevice()
{
    return m_input;
}

QList<Wayland::Surface *> Compositor::surfacesForClient(wl_client *client)
{
    QList<Wayland::Surface *> ret;

    for (int i=0; i < m_surfaces.count(); ++i) {
        if (m_surfaces.at(i)->clientHandle() == client) {
            ret.append(m_surfaces.at(i));
        }
    }
    return ret;
}

void Compositor::setRetainedSelectionWatcher(RetainedSelectionFunc func, void *param)
{
    m_retainNotify = func;
    m_retainNotifyParam = param;
}

bool Compositor::wantsRetainedSelection() const
{
    return m_retainNotify != 0;
}

void Compositor::feedRetainedSelectionData(QMimeData *data)
{
    if (m_retainNotify) {
        m_retainNotify(data, m_retainNotifyParam);
    }
}

void Compositor::overrideSelection(QMimeData *data)
{
    // ### TODO implement
}

bool Compositor::isDragging() const
{
    return false;
}

void Compositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                            Surface *surface)
{
//    Drag::instance()->dragMove(global, local, surface);
}

void Compositor::sendDragEndEvent()
{
//    Drag::instance()->dragEnd();
}

} // namespace Wayland
