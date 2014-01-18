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

#ifndef WL_COMPOSITOR_H
#define WL_COMPOSITOR_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandcompositor.h>


#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>
#include <QtGui/QWindow>

#include <private/qwldisplay_p.h>

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandInputDevice;
class QWaylandClientBufferIntegration;
class QWaylandServerBufferIntegration;
class WindowManagerServerIntegration;
class QMimeData;
class QPlatformScreenBuffer;

namespace QtWayland {

class Surface;
class SurfaceBuffer;
class InputDevice;
class DataDeviceManager;
class OutputGlobal;
class OutputExtensionGlobal;
class SurfaceExtensionGlobal;
class SubSurfaceExtensionGlobal;
class Shell;
class TouchExtensionGlobal;
class QtKeyExtensionGlobal;
class TextInputManager;
class InputPanel;
class HardwareIntegration;

class Q_COMPOSITOR_EXPORT Compositor : public QObject
{
    Q_OBJECT

public:
    Compositor(QWaylandCompositor *qt_compositor, QWaylandCompositor::ExtensionFlags extensions);
    ~Compositor();

    void frameFinished(Surface *surface = 0);

    InputDevice *defaultInputDevice(); //we just have 1 default device for now (since QPA doesn't give us anything else)

    void createSurface(struct wl_client *client, uint32_t id);
    void destroySurface(Surface *surface);
    void markSurfaceAsDirty(Surface *surface);

    void destroyClient(WaylandClient *client);

    uint currentTimeMsecs() const;

    QWindow *window() const;

    QWaylandClientBufferIntegration *clientBufferIntegration() const;
    QWaylandServerBufferIntegration *serverBufferIntegration() const;
    void initializeHardwareIntegration();
    void initializeDefaultInputDevice();
    void initializeWindowManagerProtocol();

    QList<Surface*> surfaces() const { return m_surfaces; }
    QList<Surface*> surfacesForClient(wl_client* client);
    QWaylandCompositor *waylandCompositor() const { return m_qt_compositor; }

    Surface *pickSurface(const QPointF &globalPosition);
    QPointF mapToSurface(Surface *surface, const QPointF &globalPosition);

    struct wl_display *wl_display() const { return m_display->handle(); }
    Display *display() const { return m_display; }

    static Compositor *instance();

    QList<struct wl_client *> clients() const;

    WindowManagerServerIntegration *windowManagerIntegration() const { return m_windowManagerIntegration; }

    void setScreenOrientation(Qt::ScreenOrientation orientation);
    Qt::ScreenOrientation screenOrientation() const;
    void setOutputGeometry(const QRect &geometry);
    QRect outputGeometry() const;
    void setOutputRefreshRate(int rate);
    int outputRefreshRate() const;

    Qt::ScreenOrientations orientationUpdateMaskForClient(wl_client *client);

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

    void bufferWasDestroyed(SurfaceBuffer *buffer) { m_destroyed_buffers << buffer; }
public slots:
    void cleanupGraphicsResources();

private slots:
    void processWaylandEvents();

private:
    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    QWaylandCompositor::ExtensionFlags m_extensions;

    Display *m_display;

    /* Input */
    QWaylandInputDevice *m_default_wayland_input_device;
    InputDevice *m_default_input_device;

    /* Output */
    //make this a list of the available screens
    OutputGlobal *m_output_global;

    DataDeviceManager *m_data_device_manager;

    QElapsedTimer m_timer;
    QList<Surface *> m_surfaces;
    QSet<Surface *> m_dirty_surfaces;
    QSet<Surface *> m_destroyed_surfaces;
    QSet<SurfaceBuffer *> m_destroyed_buffers;

    /* Render state */
    uint32_t m_current_frame;
    int m_last_queued_buf;

    wl_event_loop *m_loop;

    QWaylandCompositor *m_qt_compositor;
    Qt::ScreenOrientation m_orientation;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QScopedPointer<HardwareIntegration> m_hw_integration;
    QScopedPointer<QWaylandClientBufferIntegration> m_client_buffer_integration;
    QScopedPointer<QWaylandServerBufferIntegration> m_server_buffer_integration;
#endif

    //extensions
    WindowManagerServerIntegration *m_windowManagerIntegration;

    Shell *m_shell;
    OutputExtensionGlobal *m_outputExtension;
    SurfaceExtensionGlobal *m_surfaceExtension;
    SubSurfaceExtensionGlobal *m_subSurfaceExtension;
    TouchExtensionGlobal *m_touchExtension;
    QtKeyExtensionGlobal *m_qtkeyExtension;
    QScopedPointer<TextInputManager> m_textInputManager;
    QScopedPointer<InputPanel> m_inputPanel;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    bool m_retainSelection;
};

}

QT_END_NAMESPACE

#endif //WL_COMPOSITOR_H
