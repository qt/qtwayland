/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef QTCOMP_H
#define QTCOMP_H

#include <QObject>
#include <QImage>
#include <QRect>
#include <QOpenGLContext>

class QGLContext;
class QWidget;
class QMimeData;
class WaylandSurface;

namespace Wayland
{
    class Compositor;
}

class WaylandCompositor
{
public:
    WaylandCompositor(QWindow *window = 0, QOpenGLContext *context = 0, const char *socketName = 0);
    virtual ~WaylandCompositor();

    void frameFinished(WaylandSurface *surface = 0);

    void setInputFocus(WaylandSurface *surface);
    WaylandSurface *inputFocus() const;
    void destroyClientForSurface(WaylandSurface *surface);

    void setDirectRenderSurface(WaylandSurface *surface);
    WaylandSurface *directRenderSurface() const;

    QOpenGLContext *glContext() const;
    QWindow *window()const;

    virtual void surfaceCreated(WaylandSurface *surface) = 0;

    Wayland::Compositor *handle() const;

    void setRetainedSelectionEnabled(bool enable);
    virtual void retainedSelectionReceived(QMimeData *mimeData);

    const char *socketName() const;

    void setScreenOrientation(Qt::ScreenOrientation orientation);
    void setOutputGeometry(const QRect &outputGeometry);

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, WaylandSurface *surface);
    void sendDragEndEvent();

    virtual void changeCursor(const QImage &image, int hotspotX, int hotspotY);

private:
    static void retainedSelectionChanged(QMimeData *mimeData, void *param);

    Wayland::Compositor *m_compositor;
    QOpenGLContext *m_glContext;
    QWindow  *m_toplevel_widget;
    QByteArray m_socket_name;
};

#endif // QTCOMP_H
