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

#ifndef QTCOMP_H
#define QTCOMP_H

#include "waylandexport.h"

#include <QObject>
#include <QImage>
#include <QRect>

class QMimeData;
class WaylandSurface;
class WaylandInputDevice;

namespace Wayland
{
    class Compositor;
}

class Q_COMPOSITOR_EXPORT WaylandCompositor
{
public:
    WaylandCompositor(QWindow *window = 0, const char *socketName = 0);
    virtual ~WaylandCompositor();

    void frameFinished(WaylandSurface *surface = 0);

    void destroyClientForSurface(WaylandSurface *surface);
    void destroyClient(WaylandClient *client);

    QList<WaylandSurface *> surfacesForClient(WaylandClient* client) const;

    void setDirectRenderSurface(WaylandSurface *surface);
    WaylandSurface *directRenderSurface() const;

    QWindow *window()const;

    virtual void surfaceCreated(WaylandSurface *surface) = 0;
    virtual void surfaceAboutToBeDestroyed(WaylandSurface *surface);

    Wayland::Compositor *handle() const;

    void setRetainedSelectionEnabled(bool enable);
    virtual void retainedSelectionReceived(QMimeData *mimeData);
    void overrideSelection(QMimeData *data);

    void setClientFullScreenHint(bool value);

    const char *socketName() const;

    void setScreenOrientation(Qt::ScreenOrientation orientation);

    void setOutputGeometry(const QRect &outputGeometry);
    QRect outputGeometry() const;

    WaylandInputDevice *defaultInputDevice() const;

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, WaylandSurface *surface);
    void sendDragEndEvent();

    virtual void changeCursor(const QImage &image, int hotspotX, int hotspotY);

    void enableSubSurfaceExtension();

    void enableTouchExtension();
    enum TouchExtensionFlag {
        TouchExtMouseFromTouch = 0x01
    };
    Q_DECLARE_FLAGS(TouchExtensionFlags, TouchExtensionFlag)
    void configureTouchExtension(TouchExtensionFlags flags);

private:
    static void retainedSelectionChanged(QMimeData *mimeData, void *param);

    Wayland::Compositor *m_compositor;
    QWindow  *m_toplevel_window;
    QByteArray m_socket_name;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaylandCompositor::TouchExtensionFlags)

#endif // QTCOMP_H
