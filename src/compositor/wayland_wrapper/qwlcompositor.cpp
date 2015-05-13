/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlcompositor_p.h"

#include "qwaylandinput.h"
#include "qwldisplay_p.h"
#include "qwloutput_p.h"
#include "qwlsurface_p.h"
#include "qwaylandclient.h"
#include "qwaylandcompositor.h"
#include "qwldatadevicemanager_p.h"
#include "qwldatadevice_p.h"
#include "qwlextendedsurface_p.h"
#include "qwlsubsurface_p.h"
#include "qwlshellsurface_p.h"
#include "qwlqttouch_p.h"
#include "qwlqtkey_p.h"
#include "qwlinputdevice_p.h"
#include "qwlregion_p.h"
#include "qwlpointer_p.h"
#include "qwltextinputmanager_p.h"
#include <QtCompositor/QWaylandInputPanel>
#include "qwaylandview.h"
#include "qwaylandshmformathelper.h"
#include "qwaylandoutput.h"
#include "qwlkeyboard_p.h"

#include <QWindow>
#include <QSocketNotifier>
#include <QScreen>
#include <qpa/qplatformscreen.h>
#include <QGuiApplication>
#include <QDebug>

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

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

#if defined (QT_COMPOSITOR_WAYLAND_GL)
#include "hardware_integration/qwlhwintegration_p.h"
#include "hardware_integration/qwlclientbufferintegration_p.h"
#include "hardware_integration/qwlserverbufferintegration_p.h"
#endif
#include "extensions/qwaylandwindowmanagerextension.h"

#include "hardware_integration/qwlclientbufferintegrationfactory_p.h"
#include "hardware_integration/qwlserverbufferintegrationfactory_p.h"

#include "../shared/qwaylandxkb.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

static Compositor *compositor;

class WindowSystemEventHandler : public QWindowSystemEventHandler
{
public:
    WindowSystemEventHandler(Compositor *c) : compositor(c) {}
    bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type == QWindowSystemInterfacePrivate::Key) {
            QWindowSystemInterfacePrivate::KeyEvent *ke = static_cast<QWindowSystemInterfacePrivate::KeyEvent *>(e);
            Keyboard *keyb = compositor->defaultInputDevice()->keyboardDevice();

            uint32_t code = ke->nativeScanCode;
            bool isDown = ke->keyType == QEvent::KeyPress;

#ifndef QT_NO_WAYLAND_XKB
            QString text;
            Qt::KeyboardModifiers modifiers = QWaylandXkb::modifiers(keyb->xkbState());

            const xkb_keysym_t sym = xkb_state_key_get_one_sym(keyb->xkbState(), code);
            uint utf32 = xkb_keysym_to_utf32(sym);
            if (utf32)
                text = QString::fromUcs4(&utf32, 1);
            int qtkey = QWaylandXkb::keysymToQtKey(sym, modifiers, text);

            ke->key = qtkey;
            ke->modifiers = modifiers;
            ke->nativeVirtualKey = sym;
            ke->nativeModifiers = keyb->xkbModsMask();
            ke->unicode = text;
#endif
            if (!ke->repeat)
                keyb->keyEvent(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);

            QWindowSystemEventHandler::sendEvent(e);

            if (!ke->repeat) {
                keyb->updateKeymap();
                keyb->updateModifierState(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
            }
        } else {
            QWindowSystemEventHandler::sendEvent(e);
        }
        return true;
    }

    Compositor *compositor;
};

Compositor::Compositor(QWaylandCompositor *qt_compositor)
    : m_extensions(QWaylandCompositor::DefaultExtensions)
    , m_display(new Display)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_orientation(Qt::PrimaryOrientation)
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , m_hw_integration(0)
    , m_client_buffer_integration(0)
    , m_server_buffer_integration(0)
#endif
    , m_eventHandler(new WindowSystemEventHandler(this))
    , m_retainSelection(false)
    , m_initialized(false)
{
    m_outputSpaces.append(new QWaylandOutputSpace(qt_compositor));
    m_timer.start();

    QWindowSystemInterfacePrivate::installWindowSystemEventHandler(m_eventHandler.data());
}

void Compositor::init()
{
    QStringList arguments = QCoreApplication::instance()->arguments();

    int socketArg = arguments.indexOf(QLatin1String("--wayland-socket-name"));
    if (socketArg != -1 && socketArg + 1 < arguments.size())
        m_socket_name = arguments.at(socketArg + 1).toLocal8Bit();

    wl_compositor::init(m_display->handle(), 3);

    m_data_device_manager =  new DataDeviceManager(this);

    wl_display_init_shm(m_display->handle());
    QVector<wl_shm_format> formats = QWaylandShmFormatHelper::supportedWaylandFormats();
    foreach (wl_shm_format format, formats)
        wl_display_add_shm_format(m_display->handle(), format);

    const char *socketName = 0;
    if (m_socket_name.size())
        socketName = m_socket_name.constData();
    if (wl_display_add_socket(m_display->handle(), socketName)) {
        qFatal("Fatal: Failed to open server socket\n");
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(aboutToBlock()), this, SLOT(processWaylandEvents()));

    qRegisterMetaType<SurfaceBuffer*>("SurfaceBuffer*");
    qRegisterMetaType<QWaylandClient*>("WaylandClient*");
    qRegisterMetaType<QWaylandSurface*>("WaylandSurface*");
    qRegisterMetaType<QWaylandView*>("WaylandSurfaceView*");
    //initialize distancefieldglyphcache here

    initializeHardwareIntegration();
    initializeExtensions();
    initializeDefaultInputDevice();

    m_initialized = true;
}

Compositor::~Compositor()
{
    if (!m_destroyed_surfaces.isEmpty())
        qWarning("QWaylandCompositor::cleanupGraphicsResources() must be called manually");
    qDeleteAll(m_clients);

    qDeleteAll(m_outputSpaces);

    removeInputDevice(m_default_wayland_input_device);
    delete m_default_wayland_input_device;
    delete m_data_device_manager;

    delete m_display;
}

uint Compositor::currentTimeMsecs() const
{
    return m_timer.elapsed();
}

QWaylandOutput *Compositor::output(QWindow *window) const
{
    foreach (QWaylandOutputSpace *outputSpace, m_outputSpaces) {
        QWaylandOutput *output = outputSpace->output(window);
        if (output)
            return output;
    }

    return Q_NULLPTR;
}

QWaylandOutput *Compositor::primaryOutput() const
{
    return primaryOutputSpace()->primaryOutput();
}

QWaylandOutputSpace *Compositor::primaryOutputSpace() const
{
    Q_ASSERT(!m_outputSpaces.isEmpty());
    return m_outputSpaces.first();
}

void Compositor::setPrimaryOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_ASSERT(!m_outputSpaces.isEmpty());
    if (m_outputSpaces.first() == outputSpace)
        return;
    if (m_outputSpaces.removeOne(outputSpace)) {
        m_outputSpaces.prepend(outputSpace);
        waylandCompositor()->primaryOutputSpaceChanged();
    }
}

void Compositor::addOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_ASSERT(!m_outputSpaces.contains(outputSpace));
    m_outputSpaces.append(outputSpace);
    waylandCompositor()->outputSpacesChanged();
}

void Compositor::removeOutputSpace(QWaylandOutputSpace *outputSpace)
{
    if (m_outputSpaces.removeOne(outputSpace))
        waylandCompositor()->outputSpacesChanged();
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
    waylandCompositor()->surfaceAboutToBeDestroyed(surface->waylandSurface());

    m_destroyed_surfaces << surface->waylandSurface();
}

void Compositor::unregisterSurface(QWaylandSurface *surface)
{
    if (!m_all_surfaces.removeOne(surface))
        qWarning("%s Unexpected state. Cant find registered surface\n", Q_FUNC_INFO);
}

void Compositor::cleanupGraphicsResources()
{
    qDeleteAll(m_destroyed_surfaces);
    m_destroyed_surfaces.clear();
}

void Compositor::compositor_create_surface(Resource *resource, uint32_t id)
{
    QWaylandClient *client = QWaylandClient::fromWlClient(m_qt_compositor, resource->client());
    QWaylandSurface *surface = m_qt_compositor->createSurface(client, id, resource->version());
    m_all_surfaces.append(surface);
    if (primaryOutput())
        surface->setPrimaryOutput(primaryOutput());
    emit m_qt_compositor->surfaceCreated(surface);
}

void Compositor::compositor_create_region(Resource *resource, uint32_t id)
{
    new Region(resource->client(), id);
}

void Compositor::destroyClient(QWaylandClient *client)
{
    if (!client)
        return;

    QWaylandWindowManagerExtension *wmExtension = QWaylandWindowManagerExtension::findIn(waylandCompositor());
    if (wmExtension)
        wmExtension->sendQuitMessage(client->client());

    wl_client_destroy(client->client());
}

ClientBufferIntegration * Compositor::clientBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_client_buffer_integration.data();
#else
    return 0;
#endif
}

ServerBufferIntegration * Compositor::serverBufferIntegration() const
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
    if (m_extensions & QWaylandCompositor::HardwareIntegrationExtension)
        m_hw_integration.reset(new HardwareIntegration(this));

    loadClientBufferIntegration();
    loadServerBufferIntegration();

    if (m_client_buffer_integration)
        m_client_buffer_integration->initializeHardware(m_display);
    if (m_server_buffer_integration)
        m_server_buffer_integration->initializeHardware(m_qt_compositor);
#endif
}

void Compositor::initializeExtensions()
{
    if (m_extensions & QWaylandCompositor::SurfaceExtension)
        new SurfaceExtensionGlobal(this);
    if (m_extensions & QWaylandCompositor::SubSurfaceExtension)
        new SubSurfaceExtensionGlobal(waylandCompositor());
    if (m_extensions & QWaylandCompositor::TouchExtension)
        new TouchExtensionGlobal(this);
    if (m_extensions & QWaylandCompositor::QtKeyExtension)
        new QtKeyExtensionGlobal(this);
    if (m_extensions & QWaylandCompositor::TextInputExtension) {
        new TextInputManager(this);
        new QWaylandInputPanel(waylandCompositor());
    }
    if (m_extensions & QWaylandCompositor::WindowManagerExtension)
        new QWaylandWindowManagerExtension(waylandCompositor());
}

void Compositor::initializeDefaultInputDevice()
{
    m_default_wayland_input_device = m_qt_compositor->createInputDevice();
    registerInputDevice(m_default_wayland_input_device);
}

QList<QWaylandClient *> Compositor::clients() const
{
    return m_clients;
}

QWaylandCompositor::ExtensionFlags Compositor::extensions() const
{
    return m_extensions;
}

QWaylandInputDevice *Compositor::defaultInputDevice()
{
    // The list gets prepended so that default is the last element
    return m_inputDevices.last();
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
#ifdef QT_COMPOSITOR_WAYLAND_GL
    QStringList keys = ClientBufferIntegrationFactory::keys();
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
        m_client_buffer_integration.reset(ClientBufferIntegrationFactory::create(targetKey, QStringList()));
        if (m_client_buffer_integration) {
            m_client_buffer_integration->setCompositor(m_qt_compositor);
            if (m_hw_integration)
                m_hw_integration->setClientBufferIntegration(targetKey);
        }
    }
    //BUG: if there is no client buffer integration, bad things will happen when opengl is used
#endif
}

void Compositor::loadServerBufferIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    QStringList keys = ServerBufferIntegrationFactory::keys();
    QString targetKey;
    QByteArray serverBufferIntegration = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
    if (keys.contains(QString::fromLocal8Bit(serverBufferIntegration.constData()))) {
        targetKey = QString::fromLocal8Bit(serverBufferIntegration.constData());
    }
    if (!targetKey.isEmpty()) {
        m_server_buffer_integration.reset(ServerBufferIntegrationFactory::create(targetKey, QStringList()));
        if (m_hw_integration)
            m_hw_integration->setServerBufferIntegration(targetKey);
    }
#endif
}

void Compositor::registerInputDevice(QWaylandInputDevice *device)
{
    // The devices get prepended as the first input device that gets added
    // is assumed to be the default and it will claim to accept all the input
    // events if asked
    m_inputDevices.prepend(device);
}

void Compositor::removeInputDevice(QWaylandInputDevice *device)
{
    m_inputDevices.removeOne(device);
}

QWaylandInputDevice *Compositor::inputDeviceFor(QInputEvent *inputEvent)
{
    QWaylandInputDevice *dev = NULL;
    for (int i = 0; i < m_inputDevices.size(); i++) {
        QWaylandInputDevice *candidate = m_inputDevices.at(i);
        if (candidate->isOwner(inputEvent)) {
            dev = candidate;
            break;
        }
    }
    return dev;
}

} // namespace Wayland

QT_END_NAMESPACE
