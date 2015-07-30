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
#include "qwaylandsurfaceview.h"
#include "qwaylandclient.h"
#include "qwaylandkeyboard.h"
#include "qwaylandpointer.h"
#include "qwaylandtouch.h"

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"

#include "extensions/qwlinputpanel_p.h"
#include "extensions/qwlshellsurface_p.h"

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

struct wl_display *QWaylandCompositor::waylandDisplay() const
{
    return m_compositor->wl_display();
}

uint32_t QWaylandCompositor::nextSerial()
{
    return wl_display_next_serial(waylandDisplay());
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
    QList<QWaylandSurface *> surfs;
    foreach (QWaylandSurface *surface, m_compositor->m_all_surfaces) {
        if (surface->client() == client)
            surfs.append(surface);
    }
    return surfs;
}

QList<QWaylandSurface *> QWaylandCompositor::surfaces() const
{
    return m_compositor->m_all_surfaces;
}

QWaylandOutput *QWaylandCompositor::output(QWindow *window) const
{
    return m_compositor->output(window);
}

QWaylandOutput *QWaylandCompositor::primaryOutput() const
{
    return m_compositor->primaryOutput();
}

QWaylandOutputSpace *QWaylandCompositor::primaryOutputSpace() const
{
    return m_compositor->primaryOutputSpace();
}

void QWaylandCompositor::setPrimaryOutputSpace(QWaylandOutputSpace *outputSpace)
{
    m_compositor->setPrimaryOutputSpace(outputSpace);
}

void QWaylandCompositor::addOutputSpace(QWaylandOutputSpace *outputSpace)
{
    m_compositor->addOutputSpace(outputSpace);
}

void QWaylandCompositor::removeOutputSpace(QWaylandOutputSpace *outputSpace)
{
    m_compositor->removeOutputSpace(outputSpace);
}

uint QWaylandCompositor::currentTimeMsecs() const
{
    return m_compositor->currentTimeMsecs();
}

QWaylandOutput *QWaylandCompositor::createOutput(QWaylandOutputSpace *outputSpace,
                                                 QWindow *window,
                                                 const QString &manufacturer,
                                                 const QString &model)
{
    return new QWaylandOutput(outputSpace, window, manufacturer, model);
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

QWaylandInputDevice *QWaylandCompositor::createInputDevice()
{
    return new QWaylandInputDevice(this);
}

QWaylandPointer *QWaylandCompositor::createPointerDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandPointer(inputDevice);
}

QWaylandKeyboard *QWaylandCompositor::createKeyboardDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandKeyboard(inputDevice);
}

QWaylandTouch *QWaylandCompositor::createTouchDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandTouch(inputDevice);
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
    return m_compositor->defaultInputDevice();
}

QWaylandDrag *QWaylandCompositor::drag() const
{
    return m_compositor->defaultInputDevice()->drag();
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

QT_END_NAMESPACE
