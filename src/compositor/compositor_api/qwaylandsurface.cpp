/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwaylandsurface.h"

#include <private/qobject_p.h>

#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwldatadevicemanager_p.h"

#include "extensions/qwlextendedsurface_p.h"
#include "extensions/qwlsubsurface_p.h"

#include "qwaylandcompositor.h"
#include "qwaylandclient.h"
#include "qwaylandsurface_p.h"
#include "qwaylandbufferref.h"
#include "qwaylandoutput.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

QT_BEGIN_NAMESPACE

const QEvent::Type QWaylandSurfaceEnterEvent::WaylandSurfaceEnter = (QEvent::Type)QEvent::registerEventType();
const QEvent::Type QWaylandSurfaceLeaveEvent::WaylandSurfaceLeave = (QEvent::Type)QEvent::registerEventType();

QWaylandSurfacePrivate::QWaylandSurfacePrivate(wl_client *wlClient, quint32 id, int version, QWaylandCompositor *compositor, QWaylandSurface *surface)
    : QtWayland::Surface(wlClient, id, version, compositor, surface)
    , closing(false)
    , refCount(1)
    , client(QWaylandClient::fromWlClient(wlClient))
    , windowType(QWaylandSurface::None)
{}


class QWaylandSurfaceEnterEventPrivate
{
public:
    QWaylandSurfaceEnterEventPrivate(QWaylandOutput *_output)
        : output(_output)
    {
    }

    QWaylandOutput *output;
};


QWaylandSurfaceEnterEvent::QWaylandSurfaceEnterEvent(QWaylandOutput *output)
    : QEvent(WaylandSurfaceEnter)
    , d(new QWaylandSurfaceEnterEventPrivate(output))
{
}

QWaylandSurfaceEnterEvent::~QWaylandSurfaceEnterEvent()
{
    delete d;
}

QWaylandOutput *QWaylandSurfaceEnterEvent::output() const
{
    return d->output;
}


class QWaylandSurfaceLeaveEventPrivate
{
public:
    QWaylandSurfaceLeaveEventPrivate(QWaylandOutput *_output)
        : output(_output)
    {
    }

    QWaylandOutput *output;
};


QWaylandSurfaceLeaveEvent::QWaylandSurfaceLeaveEvent(QWaylandOutput *output)
    : QEvent(WaylandSurfaceLeave)
    , d(new QWaylandSurfaceLeaveEventPrivate(output))
{
}

QWaylandSurfaceLeaveEvent::~QWaylandSurfaceLeaveEvent()
{
    delete d;
}

QWaylandOutput *QWaylandSurfaceLeaveEvent::output() const
{
    return d->output;
}


QWaylandSurface::QWaylandSurface(wl_client *client, quint32 id, int version, QWaylandCompositor *compositor)
    : QObject(*new QWaylandSurfacePrivate(client, id, version, compositor, this))
{
}

QWaylandSurface::QWaylandSurface(QWaylandSurfacePrivate *dptr)
    : QObject(*dptr)
{
}

QWaylandSurface::~QWaylandSurface()
{
    Q_D(QWaylandSurface);
    d->m_compositor->unregisterSurface(this);
    d->notifyViewsAboutDestruction();
}

QWaylandClient *QWaylandSurface::client() const
{
    Q_D(const QWaylandSurface);
    if (d->isDestroyed() || !d->compositor()->clients().contains(d->client))
        return Q_NULLPTR;
    return d->client;
}

QWaylandSurface *QWaylandSurface::parentSurface() const
{
    Q_D(const QWaylandSurface);
    if (d->subSurface() && d->subSurface()->parent()) {
        return d->subSurface()->parent()->surface();
    }
    return 0;
}

QLinkedList<QWaylandSurface *> QWaylandSurface::subSurfaces() const
{
    Q_D(const QWaylandSurface);
    if (d->subSurface()) {
        return d->subSurface()->subSurfaces();
    }
    return QLinkedList<QWaylandSurface *>();
}

bool QWaylandSurface::visible() const
{
    return isMapped();
}

bool QWaylandSurface::isMapped() const
{
    Q_D(const QWaylandSurface);
    return d->mapped();
}

QSize QWaylandSurface::size() const
{
    Q_D(const QWaylandSurface);
    return d->size();
}

Qt::ScreenOrientation QWaylandSurface::contentOrientation() const
{
    Q_D(const QWaylandSurface);
    return d->contentOrientation();
}

QWaylandSurface::Origin QWaylandSurface::origin() const
{
    Q_D(const QWaylandSurface);
    return d->origin();
}

QWaylandSurface *QWaylandSurface::transientParent() const
{
    Q_D(const QWaylandSurface);
    return d->transientParent() ? d->transientParent()->waylandSurface() : 0;
}

QPointF QWaylandSurface::transientOffset() const
{
    Q_D(const QWaylandSurface);
    return d->m_transientOffset;
}

QtWayland::Surface * QWaylandSurface::handle()
{
    Q_D(QWaylandSurface);
    return d;
}

QWaylandCompositor *QWaylandSurface::compositor() const
{
    Q_D(const QWaylandSurface);
    return d->compositor()->waylandCompositor();
}

QWaylandOutput *QWaylandSurface::primaryOutput() const
{
    Q_D(const QWaylandSurface);
    if (!d->primaryOutput())
        return Q_NULLPTR;
    return d->primaryOutput()->waylandOutput();
}

void QWaylandSurface::setPrimaryOutput(QWaylandOutput *output)
{
    Q_D(QWaylandSurface);
    d->setPrimaryOutput(output->handle());
}

void QWaylandSurface::sendFrameCallbacks()
{
    Q_D(QWaylandSurface);
    d->sendFrameCallback();
}

QString QWaylandSurface::className() const
{
    Q_D(const QWaylandSurface);
    return d->className();
}

QString QWaylandSurface::title() const
{
    Q_D(const QWaylandSurface);
    return d->title();
}

bool QWaylandSurface::hasInputPanelSurface() const
{
    Q_D(const QWaylandSurface);

    return d->inputPanelSurface() != 0;
}

/*!
 * \return True if WL_SHELL_SURFACE_TRANSIENT_INACTIVE was set for this surface, meaning it should not receive keyboard focus.
 */
bool QWaylandSurface::transientInactive() const
{
    Q_D(const QWaylandSurface);
    return d->transientInactive();
}

bool QWaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const QWaylandSurface);
    return d->inputRegion().contains(p);
}

void QWaylandSurface::destroy()
{
    deref();
}

void QWaylandSurface::enter(QWaylandOutput *output)
{
    Q_D(QWaylandSurface);
    QtWayland::OutputResource *outputResource = output->handle()->outputForClient(d->resource()->client());
    if (outputResource)
        handle()->send_enter(outputResource->handle);
}

void QWaylandSurface::leave(QWaylandOutput *output)
{
    Q_D(QWaylandSurface);
    QtWayland::OutputResource *outputResource = output->handle()->outputForClient(d->resource()->client());
    if (outputResource)
        d->send_leave(outputResource->handle);
}

/*!
    Updates the surface with the compositor's retained clipboard selection. While this
    is done automatically when the surface receives keyboard focus, this function is
    useful for updating clients which do not have keyboard focus.
*/
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    const QtWayland::InputDevice *inputDevice = d->compositor()->defaultInputDevice();
    if (inputDevice) {
        const QtWayland::DataDevice *dataDevice = inputDevice->dataDevice();
        if (dataDevice) {
            d->compositor()->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}

void QWaylandSurface::ref()
{
    Q_D(QWaylandSurface);
    ++d->refCount;
}

void QWaylandSurface::deref()
{
    Q_D(QWaylandSurface);
    if (--d->refCount == 0)
        compositor()->handle()->destroySurface(d);
}

void QWaylandSurface::setMapped(bool mapped)
{
    Q_D(QWaylandSurface);
    d->setMapped(mapped);
}

QList<QWaylandSurfaceView *> QWaylandSurface::views() const
{
    Q_D(const QWaylandSurface);
    return d->views;
}

QWaylandSurface *QWaylandSurface::fromResource(::wl_resource *res)
{
    QtWayland::Surface *s = QtWayland::Surface::fromResource(res);
    if (s)
        return s->waylandSurface();
    return Q_NULLPTR;
}

QWaylandSurfacePrivate *QWaylandSurfacePrivate::get(QWaylandSurface *surface)
{
    return surface ? surface->d_func() : Q_NULLPTR;
}

void QWaylandSurfacePrivate::setTitle(const QString &title)
{
    Q_Q(QWaylandSurface);
    if (m_title != title) {
        m_title = title;
        emit q->titleChanged();
    }
}

void QWaylandSurfacePrivate::setClassName(const QString &className)
{
    Q_Q(QWaylandSurface);
    if (m_className != className) {
        m_className = className;
        emit q->classNameChanged();
    }
}

void QWaylandSurfacePrivate::refView(QWaylandSurfaceView *view)
{
    if (views.contains(view))
        return;

    views.append(view);
    waylandSurface()->ref();
}

void QWaylandSurfacePrivate::derefView(QWaylandSurfaceView *view)
{
    int nViews = views.removeAll(view);

    for (int i = 0; i < nViews && refCount > 0; i++) {
        waylandSurface()->deref();
    }
}

class QWaylandUnmapLockPrivate
{
public:
    QWaylandSurface *surface;
};

/*!
    Constructs a QWaylandUnmapLock object.

    The lock will act on the \a surface parameter, and will prevent the surface to
    be unmapped, retaining the last valid buffer when the client attachs a NULL buffer.
    The lock will be automatically released when deleted.
*/
QWaylandUnmapLock::QWaylandUnmapLock(QWaylandSurface *surface)
                 : d(new QWaylandUnmapLockPrivate)
{
    d->surface = surface;
    surface->handle()->addUnmapLock(this);
}

QWaylandUnmapLock::~QWaylandUnmapLock()
{
    d->surface->handle()->removeUnmapLock(this);
    delete d;
}

QT_END_NAMESPACE
