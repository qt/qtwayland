/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
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

#ifndef QWAYLANDCOMPOSITOR_H
#define QWAYLANDCOMPOSITOR_H

#include <QtCompositor/qwaylandexport.h>

#include <QObject>
#include <QImage>
#include <QRect>

struct wl_display;

QT_BEGIN_NAMESPACE

class QInputEvent;

class QMimeData;
class QUrl;
class QOpenGLContext;
class QWaylandClient;
class QWaylandSurface;
class QWaylandInputDevice;
class QWaylandInputPanel;
class QWaylandDrag;
class QWaylandGlobalInterface;
class QWaylandSurfaceView;
class QWaylandOutput;

namespace QtWayland
{
    class Compositor;
}

class Q_COMPOSITOR_EXPORT QWaylandCompositor
{
public:
    enum ExtensionFlag {
        WindowManagerExtension = 0x01,
        SurfaceExtension = 0x02,
        QtKeyExtension = 0x04,
        TouchExtension = 0x08,
        SubSurfaceExtension = 0x10,
        TextInputExtension = 0x20,
        HardwareIntegrationExtension = 0x40,

        DefaultExtensions = WindowManagerExtension | SurfaceExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension
    };
    Q_DECLARE_FLAGS(ExtensionFlags, ExtensionFlag)

    QWaylandCompositor(const char *socketName = Q_NULLPTR, ExtensionFlags extensions = DefaultExtensions);
    virtual ~QWaylandCompositor();

    void addGlobalInterface(QWaylandGlobalInterface *interface);
    void addDefaultShell();
    ::wl_display *waylandDisplay() const;

    void frameStarted();
    void sendFrameCallbacks(QList<QWaylandSurface *> visibleSurfaces);

    void destroyClientForSurface(QWaylandSurface *surface);
    void destroyClient(QWaylandClient *client);

    QList<QWaylandSurface *> surfacesForClient(QWaylandClient* client) const;
    QList<QWaylandSurface *> surfaces() const;

    QList<QWaylandOutput *> outputs() const;
    QWaylandOutput *output(QWindow *window);

    QWaylandOutput *primaryOutput() const;
    void setPrimaryOutput(QWaylandOutput *output);

    virtual void surfaceCreated(QWaylandSurface *surface) = 0;
    virtual void surfaceAboutToBeDestroyed(QWaylandSurface *surface);

    virtual QWaylandSurfaceView *pickView(const QPointF &globalPosition) const;
    virtual QPointF mapToView(QWaylandSurfaceView *view, const QPointF &surfacePosition) const;

    virtual bool openUrl(QWaylandClient *client, const QUrl &url);

    QtWayland::Compositor *handle() const;

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);

    void setClientFullScreenHint(bool value);

    const char *socketName() const;

#if QT_DEPRECATED_SINCE(5, 5)
    void setScreenOrientation(Qt::ScreenOrientation orientation);

    void setOutputGeometry(const QRect &outputGeometry);
    QRect outputGeometry() const;

    void setOutputRefreshRate(int refreshRate);
    int outputRefreshRate() const;
#endif

    QWaylandInputDevice *defaultInputDevice() const;

    QWaylandInputPanel *inputPanel() const;
    QWaylandDrag *drag() const;

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, QWaylandSurface *surface);
    void sendDragEndEvent();

    virtual void setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY);

    void cleanupGraphicsResources();

    enum TouchExtensionFlag {
        TouchExtMouseFromTouch = 0x01
    };
    Q_DECLARE_FLAGS(TouchExtensionFlags, TouchExtensionFlag)
    void configureTouchExtension(TouchExtensionFlags flags);

    virtual QWaylandSurfaceView *createView(QWaylandSurface *surface);

    QWaylandInputDevice *inputDeviceFor(QInputEvent *inputEvent);

protected:
    QWaylandCompositor(const char *socketName, QtWayland::Compositor *dptr);
    virtual void retainedSelectionReceived(QMimeData *mimeData);

    virtual QWaylandOutput *createOutput(QWindow *window,
                                         const QString &manufacturer,
                                         const QString &model);

    friend class QtWayland::Compositor;
    QtWayland::Compositor *m_compositor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWaylandCompositor::ExtensionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QWaylandCompositor::TouchExtensionFlags)

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITOR_H
