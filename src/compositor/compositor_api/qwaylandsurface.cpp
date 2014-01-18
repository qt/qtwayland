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

#include "qwaylandsurface.h"

#include <private/qobject_p.h>

#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlextendedsurface_p.h"
#include "wayland_wrapper/qwlsubsurface_p.h"
#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwlshellsurface_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"

#include "qwaylandcompositor.h"
#include "waylandwindowmanagerintegration.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#ifdef QT_COMPOSITOR_QUICK
#include "qwaylandsurfaceitem.h"
#include <QtQml/QQmlPropertyMap>
#endif

QT_BEGIN_NAMESPACE

class QWaylandSurfacePrivate : public QObjectPrivate
{
public:
    QWaylandSurfacePrivate(QtWayland::Surface *srfc)
        : surface(srfc)
#ifdef QT_COMPOSITOR_QUICK
        , surface_item(0)
        , windowPropertyMap(new QQmlPropertyMap)
#endif
    {}

    ~QWaylandSurfacePrivate()
    {
#ifdef QT_COMPOSITOR_QUICK
        if (surface_item)
            surface_item->setSurface(0);
        if (windowPropertyMap)
            windowPropertyMap->deleteLater();
#endif
    }

    QtWayland::Surface *surface;
#ifdef QT_COMPOSITOR_QUICK
    QWaylandSurfaceItem *surface_item;
    QQmlPropertyMap *windowPropertyMap;
#endif
};

QWaylandSurface::QWaylandSurface(QtWayland::Surface *surface)
    : QObject(*new QWaylandSurfacePrivate(surface))
{
#ifdef QT_COMPOSITOR_QUICK
    Q_D(QWaylandSurface);
    connect(this, &QWaylandSurface::windowPropertyChanged,
            d->windowPropertyMap, &QQmlPropertyMap::insert);
    connect(d->windowPropertyMap, &QQmlPropertyMap::valueChanged,
            this, &QWaylandSurface::setWindowProperty);
#endif
}

void QWaylandSurface::advanceBufferQueue()
{
    Q_D(const QWaylandSurface);
    d->surface->advanceBufferQueue();
}

WaylandClient *QWaylandSurface::client() const
{
    Q_D(const QWaylandSurface);
    return d->surface->resource()->client();
}

QWaylandSurface *QWaylandSurface::parentSurface() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->subSurface() && d->surface->subSurface()->parent()) {
        return d->surface->subSurface()->parent()->waylandSurface();
    }
    return 0;
}

QLinkedList<QWaylandSurface *> QWaylandSurface::subSurfaces() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->subSurface()) {
        return d->surface->subSurface()->subSurfaces();
    }
    return QLinkedList<QWaylandSurface *>();
}

QWaylandSurface::Type QWaylandSurface::type() const
{
    Q_D(const QWaylandSurface);
    return d->surface->type();
}

bool QWaylandSurface::isYInverted() const
{
    Q_D(const QWaylandSurface);
    return d->surface->isYInverted();
}

bool QWaylandSurface::visible() const
{
    Q_D(const QWaylandSurface);
    return d->surface->visible();
}

QPointF QWaylandSurface::pos() const
{
    Q_D(const QWaylandSurface);
    return d->surface->pos();
}

void QWaylandSurface::setPos(const QPointF &pos)
{
    Q_D(QWaylandSurface);
    d->surface->setPos(pos);
}

QSize QWaylandSurface::size() const
{
    Q_D(const QWaylandSurface);
    return d->surface->size();
}

void QWaylandSurface::requestSize(const QSize &size)
{
    Q_D(QWaylandSurface);
    if (d->surface->shellSurface())
        d->surface->shellSurface()->sendConfigure(WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT, size.width(), size.height());
}

Qt::ScreenOrientations QWaylandSurface::orientationUpdateMask() const
{
    Q_D(const QWaylandSurface);
    return d->surface->compositor()->orientationUpdateMaskForClient(static_cast<wl_client *>(client()));
}

Qt::ScreenOrientation QWaylandSurface::contentOrientation() const
{
    Q_D(const QWaylandSurface);
    if (!d->surface->extendedSurface())
        return Qt::PrimaryOrientation;
    return d->surface->extendedSurface()->contentOrientation();
}

QWaylandSurface::WindowFlags QWaylandSurface::windowFlags() const
{
    Q_D(const QWaylandSurface);
    if (!d->surface->extendedSurface())
        return QWaylandSurface::WindowFlags(0);
    return d->surface->extendedSurface()->windowFlags();
}

QWaylandSurface::WindowType QWaylandSurface::windowType() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->shellSurface())
        return d->surface->shellSurface()->windowType();
    return QWaylandSurface::None;
}

QImage QWaylandSurface::image() const
{
    Q_D(const QWaylandSurface);
    return d->surface->image();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint QWaylandSurface::texture() const
{
    Q_D(const QWaylandSurface);
    return d->surface->textureId();
}
#else //QT_COMPOSITOR_WAYLAND_GL
uint QWaylandSurface::texture() const
{
    return 0;
}
#endif

QtWayland::Surface * QWaylandSurface::handle() const
{
    Q_D(const QWaylandSurface);
    return d->surface;
}

#ifdef QT_COMPOSITOR_QUICK
QWaylandSurfaceItem *QWaylandSurface::surfaceItem() const
{
    Q_D(const QWaylandSurface);
    return d->surface_item;
}

void QWaylandSurface::setSurfaceItem(QWaylandSurfaceItem *surfaceItem)
{
    Q_D(QWaylandSurface);
    d->surface_item = surfaceItem;
}

QObject *QWaylandSurface::windowPropertyMap() const
{
    Q_D(const QWaylandSurface);
    return d->windowPropertyMap;
}

#endif //QT_COMPOSITOR_QUICK

qint64 QWaylandSurface::processId() const
{
    struct wl_client *client = static_cast<struct wl_client *>(this->client());
    pid_t pid;
    wl_client_get_credentials(client,&pid, 0,0);
    return pid;
}

QVariantMap QWaylandSurface::windowProperties() const
{
    Q_D(const QWaylandSurface);
    if (!d->surface->extendedSurface())
        return QVariantMap();

    return d->surface->extendedSurface()->windowProperties();
}

void QWaylandSurface::setWindowProperty(const QString &name, const QVariant &value)
{
    Q_D(QWaylandSurface);
    if (!d->surface->extendedSurface())
        return;

    d->surface->extendedSurface()->setWindowProperty(name, value);
}

QPointF QWaylandSurface::mapToParent(const QPointF &pos) const
{
    return pos + this->pos();
}

QPointF QWaylandSurface::mapTo(QWaylandSurface *parent, const QPointF &pos) const
{
    QPointF p = pos;
    if (parent) {
        const QWaylandSurface * surface = this;
        while (surface != parent) {
            Q_ASSERT_X(surface, "WaylandSurface::mapTo(WaylandSurface *parent, const QPoint &pos)",
                       "parent must be in parent hierarchy");
            p = surface->mapToParent(p);
            surface = surface->parentSurface();
        }
    }
    return p;
}

QWaylandCompositor *QWaylandSurface::compositor() const
{
    Q_D(const QWaylandSurface);
    return d->surface->compositor()->waylandCompositor();
}

void QWaylandSurface::frameFinished()
{
    Q_D(QWaylandSurface);
    d->surface->frameFinished();
}

QWaylandSurface *QWaylandSurface::transientParent() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->shellSurface() && d->surface->shellSurface()->transientParent())
        return d->surface->shellSurface()->transientParent()->surface()->waylandSurface();
    return 0;
}

QWindow::Visibility QWaylandSurface::visibility() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->extendedSurface())
        return d->surface->extendedSurface()->visibility();

    return QWindow::AutomaticVisibility;
}

void QWaylandSurface::setVisibility(QWindow::Visibility visibility)
{
    Q_D(QWaylandSurface);
    if (d->surface->extendedSurface())
        d->surface->extendedSurface()->setVisibility(visibility);
}

void QWaylandSurface::sendOnScreenVisibilityChange(bool visible)
{
    setVisibility(visible ? QWindow::AutomaticVisibility : QWindow::Hidden);
}

QString QWaylandSurface::className() const
{
    Q_D(const QWaylandSurface);
    return d->surface->className();
}

QString QWaylandSurface::title() const
{
    Q_D(const QWaylandSurface);
    return d->surface->title();
}

bool QWaylandSurface::hasShellSurface() const
{
    Q_D(const QWaylandSurface);
    if (d->surface->shellSurface())
        return true;

    return false;
}

bool QWaylandSurface::hasInputPanelSurface() const
{
    Q_D(const QWaylandSurface);

    return d->surface->inputPanelSurface() != 0;
}

/*!
 * \return True if WL_SHELL_SURFACE_TRANSIENT_INACTIVE was set for this surface, meaning it should not receive keyboard focus.
 */
bool QWaylandSurface::transientInactive() const
{
    Q_D(const QWaylandSurface);
    return d->surface->transientInactive();
}

void QWaylandSurface::destroySurface()
{
    Q_D(QWaylandSurface);
    if (d->surface->extendedSurface()) {
        d->surface->extendedSurface()->send_close();
    } else {
        destroySurfaceByForce();
    }
}

void QWaylandSurface::destroySurfaceByForce()
{
    Q_D(QWaylandSurface);
   wl_resource *surface_resource = d->surface->resource()->handle;
   wl_resource_destroy(surface_resource);
}

void QWaylandSurface::ping()
{
    Q_D(QWaylandSurface);
    if (d->surface->shellSurface())
        d->surface->shellSurface()->ping();
}

/*!
    Updates the surface with the compositor's retained clipboard selection. While this
    is done automatically when the surface receives keyboard focus, this function is
    useful for updating clients which do not have keyboard focus.
*/
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    const QtWayland::InputDevice *inputDevice = d->surface->compositor()->defaultInputDevice();
    if (inputDevice) {
        const QtWayland::DataDevice *dataDevice = inputDevice->dataDevice();
        if (dataDevice) {
            d->surface->compositor()->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->surface->resource()->client())->handle);
        }
    }
}

QT_END_NAMESPACE
