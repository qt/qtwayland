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

#include "qwlcompositor_p.h"

#include "qwaylandinput.h"
#include "qwldisplay_p.h"
#include "qwloutput_p.h"
#include "qwlsurface_p.h"
#include "qwaylandcompositor.h"
#include "qwldatadevicemanager_p.h"
#include "qwldatadevice_p.h"
#include "qwlextendedoutput_p.h"
#include "qwlextendedsurface_p.h"
#include "qwlsubsurface_p.h"
#include "qwlshellsurface_p.h"
#include "qwlqttouch_p.h"
#include "qwlqtkey_p.h"
#include "qwlinputdevice_p.h"
#include "qwlinputpanel_p.h"
#include "qwlregion_p.h"
#include "qwlpointer_p.h"
#include "qwltextinputmanager_p.h"

#include <QWindow>
#include <QSocketNotifier>
#include <QScreen>
#include <qpa/qplatformscreen.h>
#include <QGuiApplication>
#include <QDebug>

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

#ifdef QT_COMPOSITOR_QUICK
#include <QtQuick/QQuickWindow>
#endif

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

#include "hardware_integration/qwlhwintegration_p.h"
#include "hardware_integration/qwaylandclientbufferintegration.h"
#include "hardware_integration/qwaylandserverbufferintegration.h"
#include "waylandwindowmanagerintegration.h"

#include "hardware_integration/qwaylandclientbufferintegrationfactory.h"
#include "hardware_integration/qwaylandserverbufferintegrationfactory.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

static Compositor *compositor;

void compositor_create_surface(struct wl_client *client,
                               struct wl_resource *resource, uint32_t id)
{
     static_cast<Compositor *>(resource->data)->createSurface(client,id);
}

void compositor_create_region(struct wl_client *client,
                              struct wl_resource *compositor, uint32_t id)
{
    Q_UNUSED(compositor);
    new Region(client, id);
}

const static struct wl_compositor_interface compositor_interface = {
    compositor_create_surface,
    compositor_create_region
};

void Compositor::bind_func(struct wl_client *client, void *data,
                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_client_add_object(client,&wl_compositor_interface, &compositor_interface, id,data);
}

Compositor *Compositor::instance()
{
    return compositor;
}

Compositor::Compositor(QWaylandCompositor *qt_compositor, QWaylandCompositor::ExtensionFlags extensions)
    : m_extensions(extensions)
    , m_display(new Display)
    , m_default_input_device(0)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_orientation(Qt::PrimaryOrientation)
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , m_hw_integration(0)
    , m_client_buffer_integration(0)
    , m_server_buffer_integration(0)
#endif
    , m_windowManagerIntegration(0)
    , m_outputExtension(0)
    , m_surfaceExtension(0)
    , m_subSurfaceExtension(0)
    , m_touchExtension(0)
    , m_qtkeyExtension(0)
    , m_textInputManager()
    , m_inputPanel()
    , m_retainSelection(false)
{
    m_timer.start();
    compositor = this;

    if (extensions & QWaylandCompositor::WindowManagerExtension)
        m_windowManagerIntegration = new WindowManagerServerIntegration(qt_compositor, this);

    wl_display_add_global(m_display->handle(),&wl_compositor_interface,this,Compositor::bind_func);

    m_data_device_manager =  new DataDeviceManager(this);

    wl_display_init_shm(m_display->handle());

    m_output_global = new OutputGlobal(m_display->handle());

    m_shell = new Shell();
    wl_display_add_global(m_display->handle(), &wl_shell_interface, m_shell, Shell::bind_func);

#if defined (QT_COMPOSITOR_WAYLAND_GL)
    if (extensions & QWaylandCompositor::HardwareIntegrationExtension)
        m_hw_integration.reset(new HardwareIntegration(this));
    QWindow *window = qt_compositor->window();
    if (window && window->surfaceType() != QWindow::RasterSurface) {
        loadClientBufferIntegration();
        loadServerBufferIntegration();
    }
#endif

    if (extensions & QWaylandCompositor::OutputExtension)
        m_outputExtension = new OutputExtensionGlobal(this);
    if (extensions & QWaylandCompositor::SurfaceExtension)
        m_surfaceExtension = new SurfaceExtensionGlobal(this);
    if (extensions & QWaylandCompositor::SubSurfaceExtension)
        m_subSurfaceExtension = new SubSurfaceExtensionGlobal(this);
    if (extensions & QWaylandCompositor::TouchExtension)
        m_touchExtension = new TouchExtensionGlobal(this);
    if (extensions & QWaylandCompositor::QtKeyExtension)
        m_qtkeyExtension = new QtKeyExtensionGlobal(this);
    if (extensions & QWaylandCompositor::TextInputExtension) {
        m_textInputManager.reset(new TextInputManager(this));
        m_inputPanel.reset(new InputPanel(this));
    }

    if (wl_display_add_socket(m_display->handle(), qt_compositor->socketName())) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(aboutToBlock()), this, SLOT(processWaylandEvents()));

    qRegisterMetaType<SurfaceBuffer*>("SurfaceBuffer*");
    //initialize distancefieldglyphcache here

#ifdef QT_COMPOSITOR_QUICK
    if (QQuickWindow *w = qobject_cast<QQuickWindow *>(window)) {
        connect(w, SIGNAL(beforeSynchronizing()), this, SLOT(cleanupGraphicsResources()), Qt::DirectConnection);
    } else
#endif
    {
#if !defined(QT_NO_DEBUG) && !defined(QT_WAYLAND_NO_CLEANUP_WARNING)
        qWarning("QWaylandCompositor::cleanupGraphicsResources() must be called manually");
#endif
    }
}

Compositor::~Compositor()
{
    delete m_shell;
    delete m_outputExtension;
    delete m_surfaceExtension;
    delete m_subSurfaceExtension;
    delete m_touchExtension;
    delete m_qtkeyExtension;

    delete m_default_wayland_input_device;
    delete m_data_device_manager;

    delete m_output_global;
    delete m_display;
}

void Compositor::frameFinished(Surface *surface)
{
    if (surface && m_dirty_surfaces.contains(surface)) {
        m_dirty_surfaces.remove(surface);
        surface->sendFrameCallback();
    } else if (!surface) {
        QSet<Surface *> dirty = m_dirty_surfaces;
        m_dirty_surfaces.clear();
        foreach (Surface *surface, dirty)
            surface->sendFrameCallback();
    }
}

void Compositor::createSurface(struct wl_client *client, uint32_t id)
{
    Surface *surface = new Surface(client,id, this);

    m_surfaces << surface;
    //BUG: This may not be an on-screen window surface though
    m_qt_compositor->surfaceCreated(surface->waylandSurface());
}

uint Compositor::currentTimeMsecs() const
{
    return m_timer.elapsed();
}

void Compositor::processWaylandEvents()
{
    int ret = wl_event_loop_dispatch(m_loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
    wl_display_flush_clients(m_display->handle());
}

void Compositor::destroySurface(Surface *surface)
{
    InputDevice *dev = defaultInputDevice();
    if (dev->mouseFocus() == surface) {
        dev->setMouseFocus(0, QPointF(), QPointF());
        // Make sure the surface is reset regardless of what the grabber
        // interface's focus() does. (e.g. the default implementation does
        // nothing when a button is down which would be disastrous here)
        dev->pointerDevice()->setFocus(0, QPointF());
    }
    if (dev->pointerDevice()->current() == surface) {
        dev->pointerDevice()->setCurrent(0, QPointF());
    }
    if (dev->keyboardFocus() == surface)
        dev->setKeyboardFocus(0);

    m_surfaces.removeOne(surface);
    m_dirty_surfaces.remove(surface);

    waylandCompositor()->surfaceAboutToBeDestroyed(surface->waylandSurface());

    surface->releaseSurfaces();
    m_destroyed_surfaces << surface;
}

void Compositor::cleanupGraphicsResources()
{
    foreach (SurfaceBuffer *s, m_destroyed_buffers)
        s->bufferWasDestroyed();
    m_destroyed_buffers.clear();

    qDeleteAll(m_destroyed_surfaces);
    m_destroyed_surfaces.clear();
}

void Compositor::markSurfaceAsDirty(QtWayland::Surface *surface)
{
    m_dirty_surfaces.insert(surface);
}

void Compositor::destroyClient(WaylandClient *c)
{
    wl_client *client = static_cast<wl_client *>(c);
    if (!client)
        return;

    if (m_windowManagerIntegration)
        m_windowManagerIntegration->sendQuitMessage(client);

    wl_client_destroy(client);
}

QWindow *Compositor::window() const
{
    return m_qt_compositor->window();
}

QWaylandClientBufferIntegration * Compositor::clientBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_client_buffer_integration.data();
#else
    return 0;
#endif
}

QWaylandServerBufferIntegration * Compositor::serverBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_server_buffer_integration.data();
#else
    return 0;
#endif
}

void Compositor::initializeHardwareIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_client_buffer_integration)
        m_client_buffer_integration->initializeHardware(m_display);
    if (m_server_buffer_integration)
        m_server_buffer_integration->initializeHardware(m_qt_compositor);
#endif
}

void Compositor::initializeDefaultInputDevice()
{
    m_default_wayland_input_device = new QWaylandInputDevice(m_qt_compositor);
    m_default_input_device = m_default_wayland_input_device->handle();
}

void Compositor::initializeWindowManagerProtocol()
{
    if (m_windowManagerIntegration)
        m_windowManagerIntegration->initialize(m_display);
}

QList<struct wl_client *> Compositor::clients() const
{
    QList<struct wl_client *> list;
    foreach (Surface *surface, m_surfaces) {
        struct wl_client *client = surface->resource()->client();
        if (!list.contains(client))
            list.append(client);
    }
    return list;
}

Qt::ScreenOrientations Compositor::orientationUpdateMaskForClient(wl_client *client)
{
    Output *output = m_output_global->outputForClient(client);
    Q_ASSERT(output);
    if (output->extendedOutput)
        return output->extendedOutput->orientationUpdateMask;
    return 0;
}

void Compositor::setScreenOrientation(Qt::ScreenOrientation orientation)
{
    m_orientation = orientation;

    QList<struct wl_client*> clientList = clients();
    for (int i = 0; i < clientList.length(); ++i) {
        struct wl_client *client = clientList.at(i);
        Output *output = m_output_global->outputForClient(client);
        if (output && output->extendedOutput)
            output->extendedOutput->sendOutputOrientation(orientation);
    }
}

Qt::ScreenOrientation Compositor::screenOrientation() const
{
    return m_orientation;
}

void Compositor::setOutputGeometry(const QRect &geometry)
{
    if (m_output_global)
        m_output_global->setGeometry(geometry);
}

QRect Compositor::outputGeometry() const
{
    if (m_output_global)
        return m_output_global->geometry();
    return QRect();
}

void Compositor::setOutputRefreshRate(int rate)
{
    if (m_output_global)
        m_output_global->setRefreshRate(rate);
}

int Compositor::outputRefreshRate() const
{
    if (m_output_global)
        return m_output_global->refreshRate();
    return 0;
}

void Compositor::setClientFullScreenHint(bool value)
{
    if (m_windowManagerIntegration)
        m_windowManagerIntegration->setShowIsFullScreen(value);
}

QWaylandCompositor::ExtensionFlags Compositor::extensions() const
{
    return m_extensions;
}

InputDevice* Compositor::defaultInputDevice()
{
    return m_default_input_device;
}

QList<QtWayland::Surface *> Compositor::surfacesForClient(wl_client *client)
{
    QList<QtWayland::Surface *> ret;

    for (int i=0; i < m_surfaces.count(); ++i) {
        if (m_surfaces.at(i)->resource()->client() == client) {
            ret.append(m_surfaces.at(i));
        }
    }
    return ret;
}

Surface *Compositor::pickSurface(const QPointF &globalPosition)
{
    QWaylandSurface *surface = m_qt_compositor->pickSurface(globalPosition);
    return surface ? surface->handle() : 0;
}

QPointF Compositor::mapToSurface(Surface *surface, const QPointF &globalPosition)
{
    return m_qt_compositor->mapToSurface(surface->waylandSurface(), globalPosition);
}

void Compositor::configureTouchExtension(int flags)
{
    if (m_touchExtension)
        m_touchExtension->setFlags(flags);
}

InputPanel *Compositor::inputPanel() const
{
    return m_inputPanel.data();
}

DataDeviceManager *Compositor::dataDeviceManager() const
{
    return m_data_device_manager;
}

void Compositor::setRetainedSelectionEnabled(bool enabled)
{
    m_retainSelection = enabled;
}

bool Compositor::retainedSelectionEnabled() const
{
    return m_retainSelection;
}

void Compositor::feedRetainedSelectionData(QMimeData *data)
{
    if (m_retainSelection)
        m_qt_compositor->retainedSelectionReceived(data);
}

void Compositor::overrideSelection(const QMimeData *data)
{
    m_data_device_manager->overrideSelection(*data);
}

bool Compositor::isDragging() const
{
    return false;
}

void Compositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                            Surface *surface)
{
    Q_UNUSED(global);
    Q_UNUSED(local);
    Q_UNUSED(surface);
//    Drag::instance()->dragMove(global, local, surface);
}

void Compositor::sendDragEndEvent()
{
//    Drag::instance()->dragEnd();
}

void Compositor::loadClientBufferIntegration()
{
    QStringList keys = QWaylandClientBufferIntegrationFactory::keys();
    QString targetKey;
    QByteArray clientBufferIntegration = qgetenv("QT_WAYLAND_HARDWARE_INTEGRATION");
    if (clientBufferIntegration.isEmpty())
        clientBufferIntegration = qgetenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION");
    if (keys.contains(QString::fromLocal8Bit(clientBufferIntegration.constData()))) {
        targetKey = QString::fromLocal8Bit(clientBufferIntegration.constData());
    } else if (keys.contains(QString::fromLatin1("wayland-egl"))) {
        targetKey = QString::fromLatin1("wayland-egl");
    } else if (!keys.isEmpty()) {
        targetKey = keys.first();
    }

    if (!targetKey.isEmpty()) {
        m_client_buffer_integration.reset(QWaylandClientBufferIntegrationFactory::create(targetKey, QStringList()));
        if (m_client_buffer_integration) {
            m_client_buffer_integration->setCompositor(m_qt_compositor);
            if (m_hw_integration)
                m_hw_integration->setClientBufferIntegration(targetKey);
        }
    }
    //BUG: if there is no client buffer integration, bad things will when opengl is used
}

void Compositor::loadServerBufferIntegration()
{
    QStringList keys = QWaylandServerBufferIntegrationFactory::keys();
    QString targetKey;
    QByteArray serverBufferIntegration = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
    if (keys.contains(QString::fromLocal8Bit(serverBufferIntegration.constData()))) {
        targetKey = QString::fromLocal8Bit(serverBufferIntegration.constData());
    }
    if (!targetKey.isEmpty()) {
        m_server_buffer_integration.reset(QWaylandServerBufferIntegrationFactory::create(targetKey, QStringList()));
        if (m_hw_integration)
            m_hw_integration->setServerBufferIntegration(targetKey);
    }
}

} // namespace Wayland

QT_END_NAMESPACE
