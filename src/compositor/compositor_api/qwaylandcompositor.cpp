/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwaylandcompositor.h"

#include "qwaylandclient.h"
#include "qwaylandinput.h"
#include "qwaylandoutput.h"
#include "qwaylandglobalinterface.h"
#include "qwaylandsurfaceview.h"

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"
#include "wayland_wrapper/qwlinputpanel_p.h"
#include "wayland_wrapper/qwlshellsurface_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>

#include <QDebug>

QT_BEGIN_NAMESPACE

QWaylandCompositor::QWaylandCompositor(const char *socketName, ExtensionFlags extensions)
    : m_compositor(new QtWayland::Compositor(this, extensions))
{
    m_compositor->m_socket_name = socketName;
    m_compositor->init();
}

QWaylandCompositor::QWaylandCompositor(const char *socketName, QtWayland::Compositor *dptr)
    : m_compositor(dptr)
{
    m_compositor->m_socket_name = socketName;
    m_compositor->init();
}

QWaylandCompositor::~QWaylandCompositor()
{
    qDeleteAll(m_compositor->m_globals);
    delete m_compositor;
}

void QWaylandCompositor::addGlobalInterface(QWaylandGlobalInterface *interface)
{
    wl_global_create(m_compositor->wl_display(), interface->interface(), interface->version(), interface, QtWayland::Compositor::bindGlobal);
    m_compositor->m_globals << interface;
}

void QWaylandCompositor::addDefaultShell()
{
    addGlobalInterface(new QtWayland::Shell);
}

struct wl_display *QWaylandCompositor::waylandDisplay() const
{
    return m_compositor->wl_display();
}

void QWaylandCompositor::sendFrameCallbacks(QList<QWaylandSurface *> visibleSurfaces)
{
    m_compositor->sendFrameCallbacks(visibleSurfaces);
}

void QWaylandCompositor::frameStarted()
{
    foreach (QtWayland::Surface *surf, m_compositor->surfaces())
        surf->frameStarted();
}

void QWaylandCompositor::destroyClientForSurface(QWaylandSurface *surface)
{
    destroyClient(surface->client());
}

void QWaylandCompositor::destroyClient(QWaylandClient *client)
{
    m_compositor->destroyClient(client);
}

QList<QWaylandSurface *> QWaylandCompositor::surfacesForClient(QWaylandClient* client) const
{
    QList<QtWayland::Surface *> surfaces = m_compositor->surfaces();

    QList<QWaylandSurface *> result;

    for (int i = 0; i < surfaces.count(); ++i) {
        if (surfaces.at(i)->waylandSurface()->client() == client) {
            result.append(surfaces.at(i)->waylandSurface());
        }
    }

    return result;
}

QList<QWaylandSurface *> QWaylandCompositor::surfaces() const
{
    QList<QtWayland::Surface *> surfaces = m_compositor->surfaces();
    QList<QWaylandSurface *> surfs;
    foreach (QtWayland::Surface *s, surfaces)
        surfs << s->waylandSurface();
    return surfs;
}

QList<QWaylandOutput *> QWaylandCompositor::outputs() const
{
    return m_compositor->outputs();
}

QWaylandOutput *QWaylandCompositor::output(QWindow *window)
{
    return m_compositor->output(window);
}

QWaylandOutput *QWaylandCompositor::primaryOutput() const
{
    return m_compositor->primaryOutput();
}

void QWaylandCompositor::setPrimaryOutput(QWaylandOutput *output)
{
    m_compositor->setPrimaryOutput(output);
}

void QWaylandCompositor::cleanupGraphicsResources()
{
    m_compositor->cleanupGraphicsResources();
}

void QWaylandCompositor::surfaceAboutToBeDestroyed(QWaylandSurface *surface)
{
    Q_UNUSED(surface);
}

QWaylandSurfaceView *QWaylandCompositor::pickView(const QPointF &globalPosition) const
{
    Q_FOREACH (QWaylandOutput *output, outputs()) {
        // Skip coordinates not in output
        if (!QRectF(output->geometry()).contains(globalPosition))
            continue;

        Q_FOREACH (QWaylandSurface *surface, output->surfaces()) {
            Q_FOREACH (QWaylandSurfaceView *view, surface->views()) {
                if (QRectF(view->pos(), surface->size()).contains(globalPosition))
                    return view;
            }
        }
    }

    return Q_NULLPTR;
}

QPointF QWaylandCompositor::mapToView(QWaylandSurfaceView *surface, const QPointF &globalPosition) const
{
    return globalPosition - surface->pos();
}

/*!
    Override this to handle QDesktopServices::openUrl() requests from the clients.

    The default implementation simply forwards the request to QDesktopServices::openUrl().
*/
bool QWaylandCompositor::openUrl(QWaylandClient *client, const QUrl &url)
{
    Q_UNUSED(client);
    return QDesktopServices::openUrl(url);
}

QtWayland::Compositor * QWaylandCompositor::handle() const
{
    return m_compositor;
}

void QWaylandCompositor::setRetainedSelectionEnabled(bool enabled)
{
    m_compositor->setRetainedSelectionEnabled(enabled);
}

bool QWaylandCompositor::retainedSelectionEnabled() const
{
    return m_compositor->retainedSelectionEnabled();
}

void QWaylandCompositor::retainedSelectionReceived(QMimeData *)
{
}

void QWaylandCompositor::overrideSelection(const QMimeData *data)
{
    m_compositor->overrideSelection(data);
}

void QWaylandCompositor::setClientFullScreenHint(bool value)
{
    m_compositor->setClientFullScreenHint(value);
}

const char *QWaylandCompositor::socketName() const
{
    if (m_compositor->m_socket_name.isEmpty())
        return 0;
    return m_compositor->m_socket_name.constData();
}

#if QT_DEPRECATED_SINCE(5, 5)
/*!
  Set the screen orientation based on accelerometer data or similar.
*/
void QWaylandCompositor::setScreenOrientation(Qt::ScreenOrientation orientation)
{
    QWaylandOutput *output = primaryOutput();
    if (output) {
        bool isPortrait = output->window()->screen()->primaryOrientation() == Qt::PortraitOrientation;

        switch (orientation) {
        case Qt::PrimaryOrientation:
            output->setTransform(QWaylandOutput::TransformNormal);
            break;
        case Qt::LandscapeOrientation:
            output->setTransform(isPortrait ? QWaylandOutput::Transform270 : QWaylandOutput::TransformNormal);
            break;
        case Qt::PortraitOrientation:
            output->setTransform(isPortrait ? QWaylandOutput::TransformNormal : QWaylandOutput::Transform90);
            break;
        case Qt::InvertedLandscapeOrientation:
            output->setTransform(isPortrait ? QWaylandOutput::Transform90 : QWaylandOutput::Transform180);
            break;
        case Qt::InvertedPortraitOrientation:
            output->setTransform(isPortrait ? QWaylandOutput::Transform180 : QWaylandOutput::Transform270);
            break;
        }
    }
}

void QWaylandCompositor::setOutputGeometry(const QRect &geometry)
{
    QWaylandOutput *output = primaryOutput();
    if (output)
        output->setGeometry(geometry);
}

QRect QWaylandCompositor::outputGeometry() const
{
    QWaylandOutput *output = primaryOutput();
    if (output)
        return output->geometry();
    return QRect();
}

void QWaylandCompositor::setOutputRefreshRate(int rate)
{
    QWaylandOutput *output = primaryOutput();
    if (output)
        output->setMode({output->mode().size, rate});
}

int QWaylandCompositor::outputRefreshRate() const
{
    QWaylandOutput *output = primaryOutput();
    if (output)
        return output->mode().refreshRate;
    return 0;
}
#endif

QWaylandInputDevice *QWaylandCompositor::defaultInputDevice() const
{
    return m_compositor->defaultInputDevice()->handle();
}

QWaylandInputPanel *QWaylandCompositor::inputPanel() const
{
    return m_compositor->inputPanel()->handle();
}

QWaylandDrag *QWaylandCompositor::drag() const
{
    return m_compositor->defaultInputDevice()->dragHandle();
}

bool QWaylandCompositor::isDragging() const
{
    return m_compositor->isDragging();
}

void QWaylandCompositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                          QWaylandSurface *surface)
{
    m_compositor->sendDragMoveEvent(global, local, surface ? surface->handle() : 0);
}

void QWaylandCompositor::sendDragEndEvent()
{
    m_compositor->sendDragEndEvent();
}

void QWaylandCompositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    Q_UNUSED(surface);
    Q_UNUSED(hotspotX);
    Q_UNUSED(hotspotY);
}

void QWaylandCompositor::configureTouchExtension(TouchExtensionFlags flags)
{
    m_compositor->configureTouchExtension(flags);
}

QWaylandSurfaceView *QWaylandCompositor::createView(QWaylandSurface *surface)
{
    return new QWaylandSurfaceView(surface);
}

QWaylandInputDevice *QWaylandCompositor::inputDeviceFor(QInputEvent *inputEvent)
{
    return m_compositor->inputDeviceFor(inputEvent);
}

QWaylandOutput *QWaylandCompositor::createOutput(QWindow *window,
                                                 const QString &manufacturer,
                                                 const QString &model)
{
    return new QWaylandOutput(this, window, manufacturer, model);
}

QT_END_NAMESPACE
