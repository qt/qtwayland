// Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCOMPOSITOR_P_H
#define QWAYLANDCOMPOSITOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtCore/private/qobject_p.h>
#include <QtCore/QSet>
#include <QtCore/QElapsedTimer>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <vector>

#if QT_CONFIG(xkbcommon)
#include <QtGui/private/qxkbcommon_p.h>
#endif

QT_BEGIN_NAMESPACE

namespace QtWayland {
    class HardwareIntegration;
    class ClientBufferIntegration;
    class ServerBufferIntegration;
    class DataDeviceManager;
    class BufferManager;
}

class QWindowSystemEventHandler;
class QWaylandSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandCompositorPrivate : public QObjectPrivate, public QtWaylandServer::wl_compositor, public QtWaylandServer::wl_subcompositor
{
public:
    static QWaylandCompositorPrivate *get(QWaylandCompositor *compositor) { return compositor->d_func(); }

    QWaylandCompositorPrivate(QWaylandCompositor *compositor);
    ~QWaylandCompositorPrivate() override;

#if QT_CONFIG(xkbcommon)
    struct xkb_context *xkbContext() const { return mXkbContext.get(); }
#endif

    void preInit();
    void init();

    void destroySurface(QWaylandSurface *surface);
    void unregisterSurface(QWaylandSurface *surface);

    QWaylandOutput *defaultOutput() const { return outputs.size() ? outputs.first() : nullptr; }

    inline const QList<QtWayland::ClientBufferIntegration *> clientBufferIntegrations() const;
    inline QtWayland::ServerBufferIntegration *serverBufferIntegration() const;

#if QT_CONFIG(wayland_datadevice)
    QtWayland::DataDeviceManager *dataDeviceManager() const { return data_device_manager; }
#endif
    QtWayland::BufferManager *bufferManager() const { return buffer_manager; }
    void feedRetainedSelectionData(QMimeData *data);

    QWaylandPointer *callCreatePointerDevice(QWaylandSeat *seat)
    { return q_func()->createPointerDevice(seat); }
    QWaylandKeyboard *callCreateKeyboardDevice(QWaylandSeat *seat)
    { return q_func()->createKeyboardDevice(seat); }
    QWaylandTouch *callCreateTouchDevice(QWaylandSeat *seat)
    { return q_func()->createTouchDevice(seat); }

    inline void addClient(QWaylandClient *client);
    inline void removeClient(QWaylandClient *client);

    void addPolishObject(QObject *object);

    inline void addOutput(QWaylandOutput *output);
    inline void removeOutput(QWaylandOutput *output);

    void connectToExternalSockets();

    virtual QWaylandSeat *seatFor(QInputEvent *inputEvent);

protected:
    void compositor_create_surface(wl_compositor::Resource *resource, uint32_t id) override;
    void compositor_create_region(wl_compositor::Resource *resource, uint32_t id) override;

    void subcompositor_get_subsurface(wl_subcompositor::Resource *resource, uint32_t id, struct ::wl_resource *surface, struct ::wl_resource *parent) override;

    virtual QWaylandSurface *createDefaultSurface();
protected:
    void initializeHardwareIntegration();
    void initializeExtensions();
    void initializeSeats();

    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    QByteArray socket_name;
    QList<int> externally_added_socket_fds;
    struct wl_display *display = nullptr;
    bool ownsDisplay = false;
    QVector<QWaylandCompositor::ShmFormat> shmFormats;

    QList<QWaylandSeat *> seats;
    QList<QWaylandOutput *> outputs;

    QList<QWaylandSurface *> all_surfaces;

#if QT_CONFIG(wayland_datadevice)
    QtWayland::DataDeviceManager *data_device_manager = nullptr;
#endif
    QtWayland::BufferManager *buffer_manager = nullptr;

    QElapsedTimer timer;

    wl_event_loop *loop = nullptr;

    QList<QWaylandClient *> clients;

#if QT_CONFIG(opengl)
    bool use_hw_integration_extension = true;
    QScopedPointer<QtWayland::HardwareIntegration> hw_integration;
    QScopedPointer<QtWayland::ServerBufferIntegration> server_buffer_integration;
#endif
    QList<QtWayland::ClientBufferIntegration*> client_buffer_integrations;

    QScopedPointer<QWindowSystemEventHandler> eventHandler;

    bool retainSelection = false;
    bool preInitialized = false;
    bool initialized = false;
    std::vector<QPointer<QObject> > polish_objects;

#if QT_CONFIG(xkbcommon)
    QXkbCommon::ScopedXKBContext mXkbContext;
#endif

    Q_DECLARE_PUBLIC(QWaylandCompositor)
    Q_DISABLE_COPY(QWaylandCompositorPrivate)
};

const QList<QtWayland::ClientBufferIntegration *> QWaylandCompositorPrivate::clientBufferIntegrations() const
{
    return client_buffer_integrations;
}

QtWayland::ServerBufferIntegration * QWaylandCompositorPrivate::serverBufferIntegration() const
{
#if QT_CONFIG(opengl)
    return server_buffer_integration.data();
#else
    return 0;
#endif
}

void QWaylandCompositorPrivate::addClient(QWaylandClient *client)
{
    Q_ASSERT(!clients.contains(client));
    clients.append(client);
}

void QWaylandCompositorPrivate::removeClient(QWaylandClient *client)
{
    Q_ASSERT(clients.contains(client));
    clients.removeOne(client);
}

void QWaylandCompositorPrivate::addOutput(QWaylandOutput *output)
{
    Q_ASSERT(output);
    Q_Q(QWaylandCompositor);
    if (outputs.contains(output))
        return;
    outputs.append(output);
    emit q->outputAdded(output);
}

void QWaylandCompositorPrivate::removeOutput(QWaylandOutput *output)
{
    Q_ASSERT(output);
    Q_ASSERT(outputs.count(output) == 1);
    Q_Q(QWaylandCompositor);
    if (outputs.removeOne(output))
        emit q->outputRemoved(output);
}

QT_END_NAMESPACE

#endif //QWAYLANDCOMPOSITOR_P_H
