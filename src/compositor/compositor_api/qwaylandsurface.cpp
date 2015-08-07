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
#include "qwaylandview_p.h"
#include "qwaylandbufferref.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QWaylandSurfacePrivate::QWaylandSurfacePrivate(wl_client *wlClient, quint32 id, int version, QWaylandCompositor *compositor, QWaylandSurface *surface)
    : QtWayland::Surface(wlClient, id, version, compositor, surface)
    , closing(false)
    , refCount(1)
    , client(QWaylandClient::fromWlClient(compositor, wlClient))
{}

QWaylandSurfacePrivate::~QWaylandSurfacePrivate()
{
    for (int i = 0; i < views.size(); i++) {
        QWaylandViewPrivate::get(views.at(i))->markSurfaceAsDestroyed(q_func());
    }
    views.clear();
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

bool QWaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const QWaylandSurface);
    return d->inputRegion().contains(p);
}

void QWaylandSurface::destroy()
{
    deref();
}

void QWaylandSurface::markAsCursorSurface(bool cursorSurface)
{
    Q_D(QWaylandSurface);
    d->setCursorSurface(cursorSurface);
}

bool QWaylandSurface::isCursorSurface() const
{
    Q_D(const QWaylandSurface);
    return d->isCursorSurface();
}

/*!
    Updates the surface with the compositor's retained clipboard selection. While this
    is done automatically when the surface receives keyboard focus, this function is
    useful for updating clients which do not have keyboard focus.
*/
void QWaylandSurface::updateSelection()
{
    Q_D(QWaylandSurface);
    QWaylandInputDevice *inputDevice = d->compositor()->defaultInputDevice();
    if (inputDevice) {
        const QtWayland::DataDevice *dataDevice = QWaylandInputDevicePrivate::get(inputDevice)->dataDevice();
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

QWaylandView *QWaylandSurface::throttlingView() const
{
    Q_D(const QWaylandSurface);
    if (d->views.isEmpty())
        return Q_NULLPTR;
    return d->views.first();
}

void QWaylandSurface::setThrottlingView(QWaylandView *view)
{
    Q_D(QWaylandSurface);

    if (!view)
        return;

    int index = d->views.indexOf(view);

    if (index < 0) {
        view->setSurface(this);
        index = d->views.indexOf(view);
    }

    d->views.move(index, 0);
}

QList<QWaylandView *> QWaylandSurface::views() const
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

struct wl_resource *QWaylandSurface::resource() const
{
    Q_D(const QWaylandSurface);
    return d->resource()->handle;
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

void QWaylandSurfacePrivate::refView(QWaylandView *view)
{
    if (views.contains(view))
        return;

    views.append(view);
    waylandSurface()->ref();
}

void QWaylandSurfacePrivate::derefView(QWaylandView *view)
{
    int nViews = views.removeAll(view);

    for (int i = 0; i < nViews && refCount > 0; i++) {
        waylandSurface()->deref();
    }
}

QT_END_NAMESPACE
