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

#ifndef WL_COMPOSITOR_H
#define WL_COMPOSITOR_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandcompositor.h>

#include <QtCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>

#include <private/qwldisplay_p.h>

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

class QWaylandClient;
class QWaylandClientPrivate;
class QInputEvent;

class QWaylandCompositor;
class QWaylandInputDevice;
class WindowManagerServerIntegration;
class QMimeData;
class QPlatformScreenBuffer;
class QWaylandSurface;
class QWindowSystemEventHandler;

namespace QtWayland {

class Surface;
class SurfaceBuffer;
class InputDevice;
class DataDeviceManager;
class SurfaceExtensionGlobal;
class SubSurfaceExtensionGlobal;
class TouchExtensionGlobal;
class QtKeyExtensionGlobal;
class TextInputManager;
class InputPanel;
class HardwareIntegration;
class ClientBufferIntegration;
class ServerBufferIntegration;

class Q_COMPOSITOR_EXPORT Compositor : public QObject, public QtWaylandServer::wl_compositor
{
    Q_OBJECT

public:
    Compositor(QWaylandCompositor *qt_compositor);
    ~Compositor();

    void init();

    void sendFrameCallbacks(QList<QWaylandSurface *> visibleSurfaces);

    InputDevice *defaultInputDevice();

    void registerInputDevice(QWaylandInputDevice *device);
    QList<QWaylandInputDevice *> inputDevices() const { return m_inputDevices; }
    QWaylandInputDevice *inputDeviceFor(QInputEvent *inputEvent);
    void removeInputDevice(QWaylandInputDevice *device);

    void destroySurface(Surface *surface);

    void destroyClient(QWaylandClient *client);

    uint currentTimeMsecs() const;

    QList<QWaylandOutput *> outputs() const;
    QWaylandOutput *output(QWindow *window) const;

    void addOutput(QWaylandOutput *output);
    void removeOutput(QWaylandOutput *output);

    QWaylandOutput *primaryOutput() const;
    void setPrimaryOutput(QWaylandOutput *output);

    ClientBufferIntegration *clientBufferIntegration() const;
    ServerBufferIntegration *serverBufferIntegration() const;
    void initializeHardwareIntegration();
    void initializeExtensions();
    void initializeDefaultInputDevice();
    void initializeWindowManagerProtocol();

    QWaylandCompositor *waylandCompositor() const { return m_qt_compositor; }

    struct wl_display *wl_display() const { return m_display->handle(); }
    Display *display() const { return m_display; }

    static Compositor *instance();

    QList<QWaylandClient *> clients() const;

    WindowManagerServerIntegration *windowManagerIntegration() const { return m_windowManagerIntegration; }

    void setClientFullScreenHint(bool value);

    QWaylandCompositor::ExtensionFlags extensions() const;

    TouchExtensionGlobal *touchExtension() { return m_touchExtension; }
    void configureTouchExtension(int flags);

    QtKeyExtensionGlobal *qtkeyExtension() { return m_qtkeyExtension; }

    InputPanel *inputPanel() const;

    DataDeviceManager *dataDeviceManager() const;

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, Surface *surface);
    void sendDragEndEvent();

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);
    void feedRetainedSelectionData(QMimeData *data);

    static void bindGlobal(wl_client *client, void *data, uint32_t version, uint32_t id);
    void resetInputDevice(Surface *surface);

    void unregisterSurface(QWaylandSurface *surface);

public slots:
    void cleanupGraphicsResources();

protected:
    void compositor_create_surface(Resource *resource, uint32_t id) Q_DECL_OVERRIDE;
    void compositor_create_region(Resource *resource, uint32_t id) Q_DECL_OVERRIDE;
private slots:
    void processWaylandEvents();

protected:
    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    QWaylandCompositor::ExtensionFlags m_extensions;

    Display *m_display;
    QByteArray m_socket_name;

    /* Input */
    QWaylandInputDevice *m_default_wayland_input_device;

    QList<QWaylandInputDevice *> m_inputDevices;

    /* Output */
    QList<QWaylandOutput *> m_outputs;

    QList<QWaylandSurface *> m_all_surfaces;

    DataDeviceManager *m_data_device_manager;

    QElapsedTimer m_timer;
    QSet<QWaylandSurface *> m_destroyed_surfaces;

    /* Render state */
    uint32_t m_current_frame;
    int m_last_queued_buf;

    wl_event_loop *m_loop;

    QWaylandCompositor *m_qt_compositor;
    Qt::ScreenOrientation m_orientation;
    QList<QWaylandClient *> m_clients;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QScopedPointer<HardwareIntegration> m_hw_integration;
    QScopedPointer<ClientBufferIntegration> m_client_buffer_integration;
    QScopedPointer<ServerBufferIntegration> m_server_buffer_integration;
#endif

    //extensions
    WindowManagerServerIntegration *m_windowManagerIntegration;

    SurfaceExtensionGlobal *m_surfaceExtension;
    SubSurfaceExtensionGlobal *m_subSurfaceExtension;
    TouchExtensionGlobal *m_touchExtension;
    QtKeyExtensionGlobal *m_qtkeyExtension;
    QScopedPointer<TextInputManager> m_textInputManager;
    QScopedPointer<InputPanel> m_inputPanel;
    QList<QWaylandGlobalInterface *> m_globals;
    QScopedPointer<QWindowSystemEventHandler> m_eventHandler;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    bool m_retainSelection;
    bool m_initialized;

    friend class QT_PREPEND_NAMESPACE(QWaylandCompositor);
    friend class QT_PREPEND_NAMESPACE(QWaylandClient);
    friend class QT_PREPEND_NAMESPACE(QWaylandClientPrivate);
};

}

QT_END_NAMESPACE

#endif //WL_COMPOSITOR_H
