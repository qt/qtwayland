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

#include "waylandcompositor.h"

#include "wayland_wrapper/wlcompositor.h"
#include "wayland_wrapper/wlsurface.h"
#include "wayland_wrapper/wlselection.h"
#include <QtCore/QCoreApplication>
#include <QDebug>

#ifdef QT_COMPOSITOR_DECLARATIVE
#include "waylandsurfaceitem.h"
#endif

WaylandCompositor::WaylandCompositor(QWindow *window, QGLContext *context, const char *socketName)
    : m_compositor(0)
    , m_glContext(context)
    , m_toplevel_widget(window)
    , m_socket_name(socketName)
{
    QStringList arguments = QCoreApplication::instance()->arguments();

    int socketArg = arguments.indexOf(QLatin1String("--wayland-socket-name"));
    if (socketArg != -1 && socketArg + 1 < arguments.size())
        m_socket_name = arguments.at(socketArg + 1).toLocal8Bit();

    m_compositor = new Wayland::Compositor(this);
#ifdef QT_COMPOSITOR_DECLARATIVE
    qmlRegisterType<WaylandSurfaceItem>("WaylandCompositor", 1, 0, "WaylandSurfaceItem");
    qRegisterMetaType<WaylandSurface*>("WaylandSurface*");
#endif
    m_compositor->initializeHardwareIntegration();
    m_compositor->initializeWindowManagerProtocol();
}

WaylandCompositor::~WaylandCompositor()
{
    delete m_compositor;
}

void WaylandCompositor::frameFinished(WaylandSurface *surface)
{
    Wayland::Surface *surfaceImpl = surface? surface->handle():0;
    m_compositor->frameFinished(surfaceImpl);
}

void WaylandCompositor::setInputFocus(WaylandSurface *surface)
{
    Wayland::Surface *surfaceImpl = surface? surface->handle():0;
    m_compositor->setInputFocus(surfaceImpl);
}

WaylandSurface *WaylandCompositor::inputFocus() const
{
    Wayland::Surface *surfaceImpl = m_compositor->keyFocus();
    return surfaceImpl ? surfaceImpl->handle() : 0;
}

void WaylandCompositor::destroyClientForSurface(WaylandSurface *surface)
{
    m_compositor->destroyClientForSurface(surface->handle());
}

void WaylandCompositor::setDirectRenderSurface(WaylandSurface *surface)
{
    m_compositor->setDirectRenderSurface(surface ? surface->handle() : 0);
}

WaylandSurface *WaylandCompositor::directRenderSurface() const
{
    Wayland::Surface *surf = m_compositor->directRenderSurface();
    return surf ? surf->handle() : 0;
}

QGLContext * WaylandCompositor::glContext() const
{
    return m_glContext;
}

QWindow * WaylandCompositor::window() const
{
    return m_toplevel_widget;
}

Wayland::Compositor * WaylandCompositor::handle() const
{
    return m_compositor;
}

void WaylandCompositor::setRetainedSelectionEnabled(bool enable)
{
    Wayland::Selection *sel = Wayland::Selection::instance();
    sel->setRetainedSelection(enable);
    sel->setRetainedSelectionWatcher(retainedSelectionChanged, this);
}

void WaylandCompositor::retainedSelectionChanged(QMimeData *mimeData, void *param)
{
    WaylandCompositor *self = static_cast<WaylandCompositor *>(param);
    self->retainedSelectionReceived(mimeData);
}

void WaylandCompositor::retainedSelectionReceived(QMimeData *)
{
}

const char *WaylandCompositor::socketName() const
{
    if (m_socket_name.isEmpty())
        return 0;
    return m_socket_name.constData();
}

void WaylandCompositor::setScreenOrientation(qint32 orientationInDegrees)
{
    m_compositor->setScreenOrientation(orientationInDegrees);
}

bool WaylandCompositor::isDragging() const
{
    return m_compositor->isDragging();
}

void WaylandCompositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                          WaylandSurface *surface)
{
    m_compositor->sendDragMoveEvent(global, local, surface ? surface->handle() : 0);
}

void WaylandCompositor::sendDragEndEvent()
{
    m_compositor->sendDragEndEvent();
}

void WaylandCompositor::changeCursor(const QImage &image, int hotspotX, int hotspotY)
{
    Q_UNUSED(image);
    Q_UNUSED(hotspotX);
    Q_UNUSED(hotspotY);
    qDebug() << "changeCursor" << image.size() << hotspotX << hotspotY;
}
