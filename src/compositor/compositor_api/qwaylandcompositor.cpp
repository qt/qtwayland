/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwaylandcompositor.h"
#include "qwaylandcompositor_p.h"

#include <QtCompositor/qwaylandclient.h>
#include <QtCompositor/qwaylandinput.h>
#include <QtCompositor/qwaylandoutput.h>
#include <QtCompositor/qwaylandview.h>
#include <QtCompositor/qwaylandclient.h>
#include <QtCompositor/qwaylandkeyboard.h>
#include <QtCompositor/qwaylandpointer.h>
#include <QtCompositor/qwaylandtouch.h>

#include <QtCompositor/private/qwaylandkeyboard_p.h>
#include <QtCompositor/private/qwaylandsurface_p.h>

#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"

#include "hardware_integration/qwlhwintegration_p.h"
#include "hardware_integration/qwlclientbufferintegration_p.h"
#include "hardware_integration/qwlclientbufferintegrationfactory_p.h"
#include "hardware_integration/qwlserverbufferintegration_p.h"
#include "hardware_integration/qwlserverbufferintegrationfactory_p.h"

#include "extensions/qwaylandwindowmanagerextension.h"

#include "qwaylandxkb.h"
#include "qwaylandshmformathelper.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QSocketNotifier>

#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>

#include <QtGui/qpa/qwindowsysteminterface_p.h>
#include <QtGui/private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class WindowSystemEventHandler : public QWindowSystemEventHandler
{
public:
    WindowSystemEventHandler(QWaylandCompositor *c) : compositor(c) {}
    bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type == QWindowSystemInterfacePrivate::Key) {
            QWindowSystemInterfacePrivate::KeyEvent *ke = static_cast<QWindowSystemInterfacePrivate::KeyEvent *>(e);
            QWaylandKeyboardPrivate *keyb = QWaylandKeyboardPrivate::get(compositor->defaultInputDevice()->keyboard());

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

    QWaylandCompositor *compositor;
};

} // namespace

QWaylandCompositorPrivate::QWaylandCompositorPrivate(QWaylandCompositor *compositor)
    : display(wl_display_create())
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , hw_integration(0)
    , client_buffer_integration(0)
    , server_buffer_integration(0)
#endif
    , eventHandler(new QtWayland::WindowSystemEventHandler(compositor))
    , retainSelection(false)
    , initialized(false)
{
    timer.start();

    QWindowSystemInterfacePrivate::installWindowSystemEventHandler(eventHandler.data());
}

void QWaylandCompositorPrivate::init()
{
    Q_Q(QWaylandCompositor);
    outputSpaces.append(new QWaylandOutputSpace(q));
    QStringList arguments = QCoreApplication::instance()->arguments();

    int socketArg = arguments.indexOf(QLatin1String("--wayland-socket-name"));
    if (socketArg != -1 && socketArg + 1 < arguments.size())
        socket_name = arguments.at(socketArg + 1).toLocal8Bit();

    wl_compositor::init(display, 3);

    data_device_manager =  new QtWayland::DataDeviceManager(q);

    wl_display_init_shm(display);
    QVector<wl_shm_format> formats = QWaylandShmFormatHelper::supportedWaylandFormats();
    foreach (wl_shm_format format, formats)
        wl_display_add_shm_format(display, format);

    const char *socketName = 0;
    if (socket_name.size())
        socketName = socket_name.constData();
    if (wl_display_add_socket(display, socketName)) {
        qFatal("Fatal: Failed to open server socket\n");
    }

    loop = wl_display_get_event_loop(display);

    int fd = wl_event_loop_get_fd(loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, q);
    QObject::connect(sockNot, SIGNAL(activated(int)), q, SLOT(processWaylandEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    QObject::connect(dispatcher, SIGNAL(aboutToBlock()), q, SLOT(processWaylandEvents()));

    qRegisterMetaType<QtWayland::SurfaceBuffer*>("SurfaceBuffer*");
    qRegisterMetaType<QWaylandClient*>("WaylandClient*");
    qRegisterMetaType<QWaylandSurface*>("WaylandSurface*");
    qRegisterMetaType<QWaylandView*>("WaylandSurfaceView*");
    //initialize distancefieldglyphcache here

    initializeHardwareIntegration();
    initializeDefaultInputDevice();

    initialized = true;
}

QWaylandCompositorPrivate::~QWaylandCompositorPrivate()
{
    if (!destroyed_surfaces.isEmpty())
        qWarning("QWaylandCompositor::cleanupGraphicsResources() must be called manually");
    qDeleteAll(clients);

    qDeleteAll(outputSpaces);

    delete data_device_manager;

    wl_display_destroy(display);
}

void QWaylandCompositorPrivate::destroySurface(QWaylandSurface *surface)
{
    Q_Q(QWaylandCompositor);
    q->surfaceAboutToBeDestroyed(surface);

    destroyed_surfaces << surface;
}

void QWaylandCompositorPrivate::unregisterSurface(QWaylandSurface *surface)
{
    if (!all_surfaces.removeOne(surface))
        qWarning("%s Unexpected state. Cant find registered surface\n", Q_FUNC_INFO);
}

void QWaylandCompositorPrivate::feedRetainedSelectionData(QMimeData *data)
{
    Q_Q(QWaylandCompositor);
    if (retainSelection)
        q->retainedSelectionReceived(data);
}

void QWaylandCompositorPrivate::compositor_create_surface(Resource *resource, uint32_t id)
{
    Q_Q(QWaylandCompositor);
    QWaylandClient *client = QWaylandClient::fromWlClient(q, resource->client());
    QWaylandSurface *surface = q->createSurface(client, id, resource->version());
    all_surfaces.append(surface);
    emit q->surfaceCreated(surface);
}

void QWaylandCompositorPrivate::compositor_create_region(Resource *resource, uint32_t id)
{
    new QtWayland::Region(resource->client(), id);
}


void QWaylandCompositorPrivate::initializeHardwareIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    Q_Q(QWaylandCompositor);
    loadClientBufferIntegration();
    loadServerBufferIntegration();

    if (client_buffer_integration)
        client_buffer_integration->initializeHardware(display);
    if (server_buffer_integration)
        server_buffer_integration->initializeHardware(q);
#endif
}

void QWaylandCompositorPrivate::initializeDefaultInputDevice()
{
    Q_Q(QWaylandCompositor);
    inputDevices.append(q->createInputDevice());
}

void QWaylandCompositorPrivate::loadClientBufferIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    Q_Q(QWaylandCompositor);
    QStringList keys = QtWayland::ClientBufferIntegrationFactory::keys();
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
        client_buffer_integration.reset(QtWayland::ClientBufferIntegrationFactory::create(targetKey, QStringList()));
        if (client_buffer_integration) {
            client_buffer_integration->setCompositor(q);
            if (hw_integration)
                hw_integration->setClientBufferIntegration(targetKey);
        }
    }
    //BUG: if there is no client buffer integration, bad things will happen when opengl is used
#endif
}

void QWaylandCompositorPrivate::loadServerBufferIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    QStringList keys = QtWayland::ServerBufferIntegrationFactory::keys();
    QString targetKey;
    QByteArray serverBufferIntegration = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
    if (keys.contains(QString::fromLocal8Bit(serverBufferIntegration.constData()))) {
        targetKey = QString::fromLocal8Bit(serverBufferIntegration.constData());
    }
    if (!targetKey.isEmpty()) {
        server_buffer_integration.reset(QtWayland::ServerBufferIntegrationFactory::create(targetKey, QStringList()));
        if (hw_integration)
            hw_integration->setServerBufferIntegration(targetKey);
    }
#endif
}

QWaylandCompositor::QWaylandCompositor(QObject *parent)
    : QObject(*new QWaylandCompositorPrivate(this), parent)
{
}

QWaylandCompositor::~QWaylandCompositor()
{
}

void QWaylandCompositor::create()
{
    Q_D(QWaylandCompositor);
    d->init();
}

bool QWaylandCompositor::isCreated() const
{
    Q_D(const QWaylandCompositor);
    return d->initialized;
}

void QWaylandCompositor::setSocketName(const QByteArray &name)
{
    Q_D(QWaylandCompositor);
    if (d->initialized) {
        qWarning("%s: It is not supported to alter the compostors socket name after the compositor is initialized\n", Q_FUNC_INFO);
        return;
    }
    d->socket_name = name;
}

QByteArray QWaylandCompositor::socketName() const
{
    Q_D(const QWaylandCompositor);
    return d->socket_name;
}

struct wl_display *QWaylandCompositor::display() const
{
    Q_D(const QWaylandCompositor);
    return d->display;
}

uint32_t QWaylandCompositor::nextSerial()
{
    Q_D(QWaylandCompositor);
    return wl_display_next_serial(d->display);
}

QList<QWaylandClient *>QWaylandCompositor::clients() const
{
    Q_D(const QWaylandCompositor);
    return d->clients;
}

void QWaylandCompositor::destroyClientForSurface(QWaylandSurface *surface)
{
    destroyClient(surface->client());
}

void QWaylandCompositor::destroyClient(QWaylandClient *client)
{
    if (!client)
        return;

    QWaylandWindowManagerExtension *wmExtension = QWaylandWindowManagerExtension::findIn(this);
    if (wmExtension)
        wmExtension->sendQuitMessage(client->client());

    wl_client_destroy(client->client());
}

QList<QWaylandSurface *> QWaylandCompositor::surfacesForClient(QWaylandClient* client) const
{
    Q_D(const QWaylandCompositor);
    QList<QWaylandSurface *> surfs;
    foreach (QWaylandSurface *surface, d->all_surfaces) {
        if (surface->client() == client)
            surfs.append(surface);
    }
    return surfs;
}

QList<QWaylandSurface *> QWaylandCompositor::surfaces() const
{
    Q_D(const QWaylandCompositor);
    return d->all_surfaces;
}

QWaylandOutput *QWaylandCompositor::output(QWindow *window) const
{
    Q_D(const QWaylandCompositor);
    foreach (QWaylandOutputSpace *outputSpace, d->outputSpaces) {
        QWaylandOutput *output = outputSpace->output(window);
        if (output)
            return output;
    }

    return Q_NULLPTR;
}

QWaylandOutput *QWaylandCompositor::primaryOutput() const
{
    Q_D(const QWaylandCompositor);
    return d->primaryOutput();
}

QWaylandOutputSpace *QWaylandCompositor::primaryOutputSpace() const
{
    Q_D(const QWaylandCompositor);
    return d->primaryOutputSpace();
}

void QWaylandCompositor::setPrimaryOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandCompositor);

    Q_ASSERT(!d->outputSpaces.isEmpty());
    if (d->outputSpaces.first() == outputSpace)
        return;
    if (d->outputSpaces.removeOne(outputSpace)) {
        d->outputSpaces.prepend(outputSpace);
        primaryOutputSpaceChanged();
    }
}

void QWaylandCompositor::addOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandCompositor);
    Q_ASSERT(!d->outputSpaces.contains(outputSpace));
    d->outputSpaces.append(outputSpace);
    outputSpacesChanged();
}

void QWaylandCompositor::removeOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandCompositor);
    if (d->outputSpaces.removeOne(outputSpace))
        outputSpacesChanged();
}

uint QWaylandCompositor::currentTimeMsecs() const
{
    Q_D(const QWaylandCompositor);
    return d->timer.elapsed();
}

QWaylandOutput *QWaylandCompositor::createOutput(QWaylandOutputSpace *outputSpace,
                                                 QWindow *window,
                                                 const QString &manufacturer,
                                                 const QString &model)
{
    return new QWaylandOutput(outputSpace, window, manufacturer, model);
}

QWaylandSurface *QWaylandCompositor::createSurface(QWaylandClient *client, quint32 id, int version)
{
    return new QWaylandSurface(client, id, version, this);
}

void QWaylandCompositor::cleanupGraphicsResources()
{
    Q_D(QWaylandCompositor);
    qDeleteAll(d->destroyed_surfaces);
    d->destroyed_surfaces.clear();
}

void QWaylandCompositor::processWaylandEvents()
{
    Q_D(QWaylandCompositor);
    int ret = wl_event_loop_dispatch(d->loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
    wl_display_flush_clients(d->display);
}


QWaylandInputDevice *QWaylandCompositor::createInputDevice()
{
    return new QWaylandInputDevice(this);
}

QWaylandPointer *QWaylandCompositor::createPointerDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandPointer(inputDevice);
}

QWaylandKeyboard *QWaylandCompositor::createKeyboardDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandKeyboard(inputDevice);
}

QWaylandTouch *QWaylandCompositor::createTouchDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandTouch(inputDevice);
}

void QWaylandCompositor::setRetainedSelectionEnabled(bool enabled)
{
    Q_D(QWaylandCompositor);
    d->retainSelection = enabled;
}

bool QWaylandCompositor::retainedSelectionEnabled() const
{
    Q_D(const QWaylandCompositor);
    return d->retainSelection;
}

void QWaylandCompositor::retainedSelectionReceived(QMimeData *)
{
}

void QWaylandCompositor::overrideSelection(const QMimeData *data)
{
    Q_D(QWaylandCompositor);
    d->data_device_manager->overrideSelection(*data);
}

QWaylandInputDevice *QWaylandCompositor::defaultInputDevice() const
{
    Q_D(const QWaylandCompositor);
    return d->inputDevices.first();
}

QWaylandDrag *QWaylandCompositor::drag() const
{
    return defaultInputDevice()->drag();
}

bool QWaylandCompositor::isDragging() const
{
    return false;
}

void QWaylandCompositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                          QWaylandSurface *surface)
{
    Q_UNUSED(global);
    Q_UNUSED(local);
    Q_UNUSED(surface);
//    Drag::instance()->dragMove(global, local, surface);
}

void QWaylandCompositor::sendDragEndEvent()
{
//    Drag::instance()->dragEnd();
}

QWaylandInputDevice *QWaylandCompositor::inputDeviceFor(QInputEvent *inputEvent)
{
    Q_D(QWaylandCompositor);
    QWaylandInputDevice *dev = NULL;
    for (int i = 0; i < d->inputDevices.size(); i++) {
        QWaylandInputDevice *candidate = d->inputDevices.at(i);
        if (candidate->isOwner(inputEvent)) {
            dev = candidate;
            break;
        }
    }
    return dev;
}

QT_END_NAMESPACE
