/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QWAYLANDCOMPOSITOR_P_H
#define QWAYLANDCOMPOSITOR_P_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandOutputSpace>
#include <QtCore/private/qobject_p.h>
#include <QtCore/QSet>
#include <QtCore/QElapsedTimer>

#include <QtCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {
    class HardwareIntegration;
    class ClientBufferIntegration;
    class ServerBufferIntegration;
    class DataDeviceManager;
}

class QWindowSystemEventHandler;
class QWaylandSurface;

class Q_COMPOSITOR_EXPORT QWaylandCompositorPrivate : public QObjectPrivate, public QtWaylandServer::wl_compositor
{
public:
    static QWaylandCompositorPrivate *get(QWaylandCompositor *compositor) { return compositor->d_func(); }

    QWaylandCompositorPrivate(QWaylandCompositor *compositor);
    ~QWaylandCompositorPrivate();

    void init();

    void destroySurface(QWaylandSurface *surface);
    void unregisterSurface(QWaylandSurface *surface);

    QWaylandOutput *primaryOutput() const { return primaryOutputSpace()->primaryOutput(); }
    QWaylandOutputSpace *primaryOutputSpace() const { return outputSpaces.first(); }

    inline QtWayland::ClientBufferIntegration *clientBufferIntegration() const;
    inline QtWayland::ServerBufferIntegration *serverBufferIntegration() const;

    QtWayland::DataDeviceManager *dataDeviceManager() const { return data_device_manager; }
    void feedRetainedSelectionData(QMimeData *data);

    QWaylandOutput *callCreateOutput(QWaylandOutputSpace *outputSpace,
                                 QWindow *window,
                                 const QString &manufacturer,
                                 const QString &model)
    { return q_func()->createOutput(outputSpace, window, manufacturer, model); }
    QWaylandPointer *callCreatePointerDevice(QWaylandInputDevice *inputDevice)
    { return q_func()->createPointerDevice(inputDevice); }
    QWaylandKeyboard *callCreateKeyboardDevice(QWaylandInputDevice *inputDevice)
    { return q_func()->createKeyboardDevice(inputDevice); }
    QWaylandTouch *callCreateTouchDevice(QWaylandInputDevice *inputDevice)
    { return q_func()->createTouchDevice(inputDevice); }

    inline void addClient(QWaylandClient *client);
    inline void removeClient(QWaylandClient *client);
protected:
    void compositor_create_surface(Resource *resource, uint32_t id) Q_DECL_OVERRIDE;
    void compositor_create_region(Resource *resource, uint32_t id) Q_DECL_OVERRIDE;

protected:
    void initializeHardwareIntegration();
    void initializeExtensions();
    void initializeDefaultInputDevice();

    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    QByteArray socket_name;
    struct wl_display *display;

    QList<QWaylandInputDevice *> inputDevices;

    QList<QWaylandOutputSpace *> outputSpaces;
    QList<QWaylandSurface *> all_surfaces;
    QSet<QWaylandSurface *> destroyed_surfaces;

    QtWayland::DataDeviceManager *data_device_manager;

    QElapsedTimer timer;

    wl_event_loop *loop;

    QList<QWaylandClient *> clients;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QScopedPointer<QtWayland::HardwareIntegration> hw_integration;
    QScopedPointer<QtWayland::ClientBufferIntegration> client_buffer_integration;
    QScopedPointer<QtWayland::ServerBufferIntegration> server_buffer_integration;
#endif

    QScopedPointer<QWindowSystemEventHandler> eventHandler;

    bool retainSelection;
    bool initialized;

    Q_DECLARE_PUBLIC(QWaylandCompositor)
    Q_DISABLE_COPY(QWaylandCompositorPrivate)
};

QtWayland::ClientBufferIntegration * QWaylandCompositorPrivate::clientBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return client_buffer_integration.data();
#else
    return 0;
#endif
}

QtWayland::ServerBufferIntegration * QWaylandCompositorPrivate::serverBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
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

QT_END_NAMESPACE

#endif //QWAYLANDCOMPOSITOR_P_H
