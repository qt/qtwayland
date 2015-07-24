/****************************************************************************
**
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

    QWaylandCompositor(const char *socketName = 0, ExtensionFlags extensions = DefaultExtensions);
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
