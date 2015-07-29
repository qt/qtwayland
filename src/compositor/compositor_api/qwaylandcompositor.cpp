/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwaylandcompositor.h"

#include "qwaylandclient.h"
#include "qwaylandinput.h"
#include "qwaylandoutput.h"
#include "qwaylandglobalinterface.h"
#include "qwaylandsurfaceview.h"
#include "qwaylandclient.h"

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

QWaylandCompositor::QWaylandCompositor(QObject *parent)
    : QObject(parent)
    , m_compositor(new QtWayland::Compositor(this))
{
}

QWaylandCompositor::~QWaylandCompositor()
{
    qDeleteAll(m_compositor->m_globals);
    delete m_compositor;
}

void QWaylandCompositor::create()
{
    m_compositor->init();
}

bool QWaylandCompositor::isCreated() const
{
    return m_compositor->m_initialized;
}

void QWaylandCompositor::setSocketName(const QByteArray &name)
{
    if (m_compositor->m_initialized) {
        qWarning("%s: It is not supported to alter the compostors socket name after the compositor is initialized\n", Q_FUNC_INFO);
        return;
    }
    m_compositor->m_socket_name = name;
}

QByteArray QWaylandCompositor::socketName() const
{
    return m_compositor->m_socket_name;
}

void QWaylandCompositor::setExtensionFlags(QWaylandCompositor::ExtensionFlags flags)
{
    if (m_compositor->m_initialized) {
        qWarning("%s: It is not supported to alter the extension flags after the compositor is initialized\n", Q_FUNC_INFO);
        return;
    }
    m_compositor->m_extensions = flags;
}

QWaylandCompositor::ExtensionFlags QWaylandCompositor::extensionFlags() const
{
    return m_compositor->extensions();
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

void QWaylandCompositor::destroyClientForSurface(QWaylandSurface *surface)
{
    destroyClient(surface->client());
}

void QWaylandCompositor::destroyClient(QWaylandClient *client)
{
    m_compositor->destroyClient(client);
}

#if QT_DEPRECATED_SINCE(5, 5)
void QWaylandCompositor::frameStarted()
{
    foreach (QWaylandOutput *output, outputs())
        output->frameStarted();
}

void QWaylandCompositor::sendFrameCallbacks(QList<QWaylandSurface *> visibleSurfaces)
{
    Q_FOREACH (QWaylandSurface *surface, visibleSurfaces) {
        surface->handle()->sendFrameCallback();
    }
}

QList<QWaylandSurface *> QWaylandCompositor::surfacesForClient(QWaylandClient* client) const
{
    QList<QWaylandSurface *> surfs;
    foreach (QWaylandOutput *output, outputs())
        surfs.append(output->surfacesForClient(client));
    return surfs;
}

QList<QWaylandSurface *> QWaylandCompositor::surfaces() const
{
    QList<QWaylandSurface *> surfs;
    foreach (QWaylandOutput *output, outputs()) {
        foreach (QWaylandSurface *surface, output->surfaces()) {
            if (!surfs.contains(surface))
                surfs.append(surface);
        }
    }
    return surfs;
}
#endif //QT_DEPRECATED_SINCE(5, 5)

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

QWaylandSurface *QWaylandCompositor::createSurface(QWaylandClient *client, quint32 id, int version)
{
    return new QWaylandSurface(client->client(), id, version, this);
}

void QWaylandCompositor::cleanupGraphicsResources()
{
    m_compositor->cleanupGraphicsResources();
}

QWaylandSurfaceView *QWaylandCompositor::createView()
{
    return new QWaylandSurfaceView();
}

QWaylandSurfaceView *QWaylandCompositor::pickView(const QPointF &globalPosition) const
{
    Q_FOREACH (QWaylandOutput *output, outputs()) {
        // Skip coordinates not in output
        if (!QRectF(output->geometry()).contains(globalPosition))
            continue;
        Q_FOREACH (QWaylandSurface *surface, output->surfaces()) {
            Q_FOREACH (QWaylandSurfaceView *view, surface->views()) {
                if (QRectF(view->requestedPosition(), surface->size()).contains(globalPosition))
                    return view;
            }
        }
    }

    return Q_NULLPTR;
}

QPointF QWaylandCompositor::mapToView(QWaylandSurfaceView *surface, const QPointF &globalPosition) const
{
    return globalPosition - surface->requestedPosition();
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

#if QT_DEPRECATED_SINCE(5, 5)
void QWaylandCompositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    Q_UNUSED(surface);
    Q_UNUSED(hotspotX);
    Q_UNUSED(hotspotY);
}
#endif

void QWaylandCompositor::configureTouchExtension(TouchExtensionFlags flags)
{
    m_compositor->configureTouchExtension(flags);
}


QWaylandSurfaceView *QWaylandCompositor::createSurfaceView(QWaylandSurface *surface)
{
    QWaylandSurfaceView *view = createView();
    view->setSurface(surface);
    return view;
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
