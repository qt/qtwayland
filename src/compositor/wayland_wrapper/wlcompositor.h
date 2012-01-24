/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#include "waylandexport.h"

#include <QtCore/QSet>

#include "wloutput.h"
#include "wldisplay.h"
#include "wlshmbuffer.h"

#include <wayland-server.h>

class WaylandCompositor;
class GraphicsHardwareIntegration;
class WindowManagerServerIntegration;
class QMimeData;

namespace Wayland {

class Surface;
class InputDevice;
class DataDeviceManager;
class OutputExtensionGlobal;
class SurfaceExtensionGlobal;
class SubSurfaceExtensionGlobal;
class Shell;
class TouchExtensionGlobal;

class Q_COMPOSITOR_EXPORT Compositor : public QObject
{
    Q_OBJECT

public:
    Compositor(WaylandCompositor *qt_compositor);
    ~Compositor();

    void frameFinished(Surface *surface = 0);

    Surface *getSurfaceFromWinId(uint winId) const;
    struct wl_client *getClientFromWinId(uint winId) const;
    QImage image(uint winId) const;

    InputDevice *defaultInputDevice(); //we just have 1 default device for now (since QPA doesn't give us anything else)

    void createSurface(struct wl_client *client, uint32_t id);
    void surfaceDestroyed(Surface *surface);
    void markSurfaceAsDirty(Surface *surface);

    void destroyClientForSurface(Surface *surface);

    static uint currentTimeMsecs();

    QWindow *window() const;

    GraphicsHardwareIntegration *graphicsHWIntegration() const;
    void initializeHardwareIntegration();
    void initializeDefaultInputDevice();
    void initializeWindowManagerProtocol();
    void enableSubSurfaceExtension();
    bool setDirectRenderSurface(Surface *surface);
    Surface *directRenderSurface() const {return m_directRenderSurface;}

    QList<Surface*> surfacesForClient(wl_client* client);

    WaylandCompositor *qtCompositor() const { return m_qt_compositor; }

    struct wl_display *wl_display() const { return m_display->handle(); }

    static Compositor *instance();

    QList<struct wl_client *> clients() const;

    WindowManagerServerIntegration *windowManagerIntegration() const { return m_windowManagerIntegration; }

    void setScreenOrientation(Qt::ScreenOrientation orientation);
    Qt::ScreenOrientation screenOrientation() const;
    void setOutputGeometry(const QRect &geometry);

    void enableTouchExtension();
    TouchExtensionGlobal *touchExtension() { return m_touchExtension; }

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, Surface *surface);
    void sendDragEndEvent();

    typedef void (*RetainedSelectionFunc)(QMimeData *, void *);
    void setRetainedSelectionWatcher(RetainedSelectionFunc func, void *param);
    void overrideSelection(QMimeData *data);

    bool wantsRetainedSelection() const;
    void feedRetainedSelectionData(QMimeData *data);

public slots:
    void releaseBuffer(void*);

private slots:
    void processWaylandEvents();

private:
    Display *m_display;

    /* Input */
    InputDevice *m_default_input_device;

    /* Output */
    //make this a list of the available screens
    OutputGlobal m_output_global;
    /* shm/*/
    ShmHandler m_shm;

    DataDeviceManager *m_data_device_manager;

    QList<Surface *> m_surfaces;
    QSet<Surface *> m_dirty_surfaces;

    /* Render state */
    uint32_t m_current_frame;
    int m_last_queued_buf;

    wl_event_loop *m_loop;

    WaylandCompositor *m_qt_compositor;
    Qt::ScreenOrientation m_orientation;

    Surface *m_directRenderSurface;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GraphicsHardwareIntegration *m_graphics_hw_integration;
#endif

    //extensions
    WindowManagerServerIntegration *m_windowManagerIntegration;


    Shell *m_shell;
    OutputExtensionGlobal *m_outputExtension;
    SurfaceExtensionGlobal *m_surfaceExtension;
    SubSurfaceExtensionGlobal *m_subSurfaceExtension;
    TouchExtensionGlobal *m_touchExtension;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    RetainedSelectionFunc m_retainNotify;
    void *m_retainNotifyParam;
};

}

#endif //WL_COMPOSITOR_H
