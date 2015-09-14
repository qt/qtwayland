/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include "qwaylandoutput.h"
#include "qwaylandoutput_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandView>

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>
#include <QtWaylandCompositor/private/qwaylandoutputspace_p.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>
#include <QtGui/QWindow>
#include <QtGui/QExposeEvent>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

static QtWaylandServer::wl_output::subpixel toWlSubpixel(const QWaylandOutput::Subpixel &value)
{
    switch (value) {
    case QWaylandOutput::SubpixelUnknown:
        return QtWaylandServer::wl_output::subpixel_unknown;
    case QWaylandOutput::SubpixelNone:
        return QtWaylandServer::wl_output::subpixel_none;
    case QWaylandOutput::SubpixelHorizontalRgb:
        return QtWaylandServer::wl_output::subpixel_horizontal_rgb;
    case QWaylandOutput::SubpixelHorizontalBgr:
        return QtWaylandServer::wl_output::subpixel_horizontal_bgr;
    case QWaylandOutput::SubpixelVerticalRgb:
        return QtWaylandServer::wl_output::subpixel_vertical_rgb;
    case QWaylandOutput::SubpixelVerticalBgr:
        return QtWaylandServer::wl_output::subpixel_vertical_bgr;
    default:
        break;
    }

    return QtWaylandServer::wl_output::subpixel_unknown;
}

static QtWaylandServer::wl_output::transform toWlTransform(const QWaylandOutput::Transform &value)
{
    switch (value) {
    case QWaylandOutput::Transform90:
        return QtWaylandServer::wl_output::transform_90;
    case QWaylandOutput::Transform180:
        return QtWaylandServer::wl_output::transform_180;
    case QWaylandOutput::Transform270:
        return QtWaylandServer::wl_output::transform_270;
    case QWaylandOutput::TransformFlipped:
        return QtWaylandServer::wl_output::transform_flipped;
    case QWaylandOutput::TransformFlipped90:
        return QtWaylandServer::wl_output::transform_flipped_90;
    case QWaylandOutput::TransformFlipped180:
        return QtWaylandServer::wl_output::transform_flipped_180;
    case QWaylandOutput::TransformFlipped270:
        return QtWaylandServer::wl_output::transform_flipped_270;
    default:
        break;
    }

    return QtWaylandServer::wl_output::transform_normal;
}

QWaylandOutputPrivate::QWaylandOutputPrivate()
    : QtWaylandServer::wl_output()
    , outputSpace(Q_NULLPTR)
    , window(Q_NULLPTR)
    , subpixel(QWaylandOutput::SubpixelUnknown)
    , transform(QWaylandOutput::TransformNormal)
    , scaleFactor(1)
    , sizeFollowsWindow(true)
    , initialized(false)
{
    mode.size = QSize();
    mode.refreshRate = 60;

    qRegisterMetaType<QWaylandOutput::Mode>("WaylandOutput::Mode");
}

QWaylandOutputPrivate::~QWaylandOutputPrivate()
{
    Q_Q(QWaylandOutput);
    if (outputSpace) {
        QWaylandOutputSpacePrivate::get(outputSpace)->removeOutput(q);
        outputSpace = Q_NULLPTR;
    }
}

void QWaylandOutputPrivate::output_bind_resource(Resource *resource)
{
    send_geometry(resource->handle,
                  position.x(), position.y(),
                  physicalSize.width(), physicalSize.height(),
                  toWlSubpixel(subpixel), manufacturer, model,
                  toWlTransform(transform));

    send_mode(resource->handle, mode_current | mode_preferred,
              mode.size.width(), mode.size.height(),
              mode.refreshRate);

    if (resource->version() >= 2) {
        send_scale(resource->handle, scaleFactor);
        send_done(resource->handle);
    }
}

void QWaylandOutputPrivate::sendGeometryInfo()
{
    Q_FOREACH (Resource *resource, resourceMap().values()) {
        send_geometry(resource->handle,
                      position.x(), position.x(),
                      physicalSize.width(), physicalSize.height(),
                      toWlSubpixel(subpixel), manufacturer, model,
                      toWlTransform(transform));
        if (resource->version() >= 2)
            send_done(resource->handle);
    }
}


void QWaylandOutputPrivate::addView(QWaylandView *view, QWaylandSurface *surface)
{
    for (int i = 0; i < surfaceViews.size(); i++) {
        if (surface == surfaceViews.at(i).surface) {
            if (!surfaceViews.at(i).views.contains(view)) {
                surfaceViews[i].views.append(view);
            }
            return;
        }
    }

    surfaceViews.append(QWaylandSurfaceViewMapper(surface,view));
}

void QWaylandOutputPrivate::removeView(QWaylandView *view, QWaylandSurface *surface)
{
    Q_Q(QWaylandOutput);
    for (int i = 0; i < surfaceViews.size(); i++) {
        if (surface == surfaceViews.at(i).surface) {
            bool removed = surfaceViews[i].views.removeOne(view);
            if (surfaceViews.at(i).views.isEmpty() && removed) {
                if (surfaceViews.at(i).has_entered)
                    q->surfaceLeave(surface);
                surfaceViews.remove(i);
            }
            return;
        }
    }
    qWarning("%s Could not find view %p for surface %p to remove. Possible invalid state", Q_FUNC_INFO, view, surface);
}

QWaylandOutput::QWaylandOutput()
    : QObject(*new QWaylandOutputPrivate())
{
}

QWaylandOutput::QWaylandOutput(QWaylandOutputSpace *outputSpace, QWindow *window)
    : QObject(*new QWaylandOutputPrivate())
{
    Q_D(QWaylandOutput);
    d->outputSpace = outputSpace;
    d->window = window;
    QWaylandCompositorPrivate::get(outputSpace->compositor())->addPolishObject(this);
}

QWaylandOutput::~QWaylandOutput()
{
}

void QWaylandOutput::initialize()
{
    Q_D(QWaylandOutput);

    Q_ASSERT(!d->initialized);
    Q_ASSERT(d->outputSpace);
    Q_ASSERT(d->window);
    Q_ASSERT(d->outputSpace->compositor());
    Q_ASSERT(d->outputSpace->compositor()->isCreated());

    d->mode.size = d->window->size();

    QWaylandOutputSpacePrivate::get(d->outputSpace)->addOutput(this);

    QObject::connect(d->window, &QWindow::widthChanged, this, &QWaylandOutput::setWidth);
    QObject::connect(d->window, &QWindow::heightChanged, this, &QWaylandOutput::setHeight);
    QObject::connect(d->window, &QObject::destroyed, this, &QWaylandOutput::handleWindowDestroyed);

    d->init(d->compositor()->display(), 2);

    d->initialized = true;
}

QWaylandOutput *QWaylandOutput::fromResource(wl_resource *resource)
{
    return static_cast<QWaylandOutputPrivate *>(QWaylandOutputPrivate::Resource::fromResource(resource)->output_object)->q_func();
}

struct ::wl_resource *QWaylandOutput::resourceForClient(QWaylandClient *client) const
{
    Q_D(const QWaylandOutput);
    QWaylandOutputPrivate::Resource *r = d->resourceMap().value(client->client());
    if (r)
        return r->handle;

    return Q_NULLPTR;
}

QWaylandOutputSpace *QWaylandOutput::outputSpace() const
{
    return d_func()->outputSpace;
}

void QWaylandOutput::setOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandOutput);

    if (d->outputSpace == outputSpace)
        return;

    if (d->initialized) {
        qWarning("Setting QWaylandOutputSpace %p on QWaylandOutput %p is not supported after QWaylandOutput has been initialized\n", outputSpace, this);
        return;
    }
    if (d->outputSpace && d->outputSpace->compositor() != outputSpace->compositor()) {
        qWarning("Possible initialization error. Moving QWaylandOutput %p between compositor instances.\n", this);
    }

    if (!d->outputSpace) {
        QWaylandCompositorPrivate::get(outputSpace->compositor())->addPolishObject(this);
    }
    d->outputSpace = outputSpace;
    emit outputSpaceChanged();
}

void QWaylandOutput::update()
{
    QRect rect(QPoint(0, 0), window()->size());
    QRegion region(rect);
    QExposeEvent *event = new QExposeEvent(region);
    QCoreApplication::postEvent(window(), event);
}

QWaylandCompositor *QWaylandOutput::compositor() const
{
    return d_func()->compositor();
}

QString QWaylandOutput::manufacturer() const
{
    return d_func()->manufacturer;
}

void QWaylandOutput::setManufacturer(const QString &manufacturer)
{
    d_func()->manufacturer = manufacturer;
}

QString QWaylandOutput::model() const
{
    return d_func()->model;
}

void QWaylandOutput::setModel(const QString &model)
{
    d_func()->model = model;
}

QPoint QWaylandOutput::position() const
{
    return d_func()->position;
}

void QWaylandOutput::setPosition(const QPoint &pt)
{
    Q_D(QWaylandOutput);
    if (d->position == pt)
        return;

    d->position = pt;

    d->sendGeometryInfo();

    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();
}

QWaylandOutput::Mode QWaylandOutput::mode() const
{
    return d_func()->mode;
}

void QWaylandOutput::setMode(const Mode &mode)
{
    Q_D(QWaylandOutput);
    if (d->mode.size == mode.size && d->mode.refreshRate == mode.refreshRate)
        return;

    d->mode = mode;

    Q_FOREACH (QWaylandOutputPrivate::Resource *resource, d->resourceMap().values()) {
        d->send_mode(resource->handle, d->mode_current,
                  d->mode.size.width(), d->mode.size.height(),
                  d->mode.refreshRate * 1000);
        if (resource->version() >= 2)
            d->send_done(resource->handle);
    }

    Q_EMIT modeChanged();
    Q_EMIT geometryChanged();

    if (d->window) {
        d->window->resize(mode.size);
        d->window->setMinimumSize(mode.size);
        d->window->setMaximumSize(mode.size);
    }
}

QRect QWaylandOutput::geometry() const
{
    Q_D(const QWaylandOutput);
    return QRect(d->position, d->mode.size);
}

void QWaylandOutput::setGeometry(const QRect &geometry)
{
    Q_D(QWaylandOutput);
    if (d->position == geometry.topLeft() && d->mode.size == geometry.size())
        return;

    d->position = geometry.topLeft();
    d->mode.size = geometry.size();

    Q_FOREACH (QWaylandOutputPrivate::Resource *resource, d->resourceMap().values()) {
        d->send_geometry(resource->handle,
                      d->position.x(), d->position.y(),
                      d->physicalSize.width(), d->physicalSize.height(),
                      toWlSubpixel(d->subpixel), d->manufacturer, d->model,
                      toWlTransform(d->transform));
        d->send_mode(resource->handle, d->mode_current,
                  d->mode.size.width(), d->mode.size.height(),
                  d->mode.refreshRate * 1000);
        if (resource->version() >= 2)
            d->send_done(resource->handle);
    }
    Q_EMIT positionChanged();
    Q_EMIT modeChanged();

    if (window()) {
        window()->resize(geometry.size());
        window()->setMinimumSize(geometry.size());
        window()->setMaximumSize(geometry.size());
    }
}

QRect QWaylandOutput::availableGeometry() const
{
    Q_D(const QWaylandOutput);
    if (!d->availableGeometry.isValid())
        return QRect(d->position, d->mode.size);

    return d->availableGeometry;
}

void QWaylandOutput::setAvailableGeometry(const QRect &availableGeometry)
{
    Q_D(QWaylandOutput);
    if (d->availableGeometry == availableGeometry)
        return;

    d->availableGeometry = availableGeometry;

    Q_EMIT availableGeometryChanged();
}

QSize QWaylandOutput::physicalSize() const
{
    return d_func()->physicalSize;
}

void QWaylandOutput::setPhysicalSize(const QSize &size)
{
    Q_D(QWaylandOutput);
    if (d->physicalSize == size)
        return;

    d->physicalSize = size;

    d->sendGeometryInfo();

    Q_EMIT physicalSizeChanged();
}

QWaylandOutput::Subpixel QWaylandOutput::subpixel() const
{
    return d_func()->subpixel;
}

void QWaylandOutput::setSubpixel(const Subpixel &subpixel)
{
    Q_D(QWaylandOutput);
    if (d->subpixel == subpixel)
        return;

    d->subpixel = subpixel;

    d->sendGeometryInfo();

    Q_EMIT subpixelChanged();
}

QWaylandOutput::Transform QWaylandOutput::transform() const
{
    return d_func()->transform;
}

void QWaylandOutput::setTransform(const Transform &transform)
{
    Q_D(QWaylandOutput);
    if (d->transform == transform)
        return;

    d->transform = transform;

    d->sendGeometryInfo();

    Q_EMIT transformChanged();
}

int QWaylandOutput::scaleFactor() const
{
    return d_func()->scaleFactor;
}

void QWaylandOutput::setScaleFactor(int scale)
{
    Q_D(QWaylandOutput);
    if (d->scaleFactor == scale)
        return;

    d->scaleFactor = scale;

    Q_FOREACH (QWaylandOutputPrivate::Resource *resource, d->resourceMap().values()) {
        if (resource->version() >= 2) {
            d->send_scale(resource->handle, scale);
            d->send_done(resource->handle);
        }
    }

    Q_EMIT scaleFactorChanged();
}

bool QWaylandOutput::sizeFollowsWindow() const
{
    return d_func()->sizeFollowsWindow;
}

void QWaylandOutput::setSizeFollowsWindow(bool follow)
{
    Q_D(QWaylandOutput);
    if (follow != d->sizeFollowsWindow) {
        if (follow) {
            QObject::connect(d->window, &QWindow::widthChanged, this, &QWaylandOutput::setWidth);
            QObject::connect(d->window, &QWindow::heightChanged, this, &QWaylandOutput::setHeight);
        } else {
            QObject::disconnect(d->window, &QWindow::widthChanged, this, &QWaylandOutput::setWidth);
            QObject::disconnect(d->window, &QWindow::heightChanged, this, &QWaylandOutput::setHeight);
        }
        d->sizeFollowsWindow = follow;
        Q_EMIT sizeFollowsWindowChanged();
    }
}

QWindow *QWaylandOutput::window() const
{
    return d_func()->window;
}

void QWaylandOutput::setWindow(QWindow *window)
{
    Q_D(QWaylandOutput);
    if (d->window == window)
        return;
    if (d->initialized) {
        qWarning("Setting QWindow %p on QWaylandOutput %p is not supported after QWaylandOutput has been initialized\n", window, this);
        return;
    }
    d->window = window;
    emit windowChanged();
}

void QWaylandOutput::frameStarted()
{
    Q_D(QWaylandOutput);
    for (int i = 0; i < d->surfaceViews.size(); i++) {
        QWaylandSurfaceViewMapper &surfacemapper = d->surfaceViews[i];
        if (surfacemapper.maybeThrottelingView())
            surfacemapper.surface->frameStarted();
    }
}

void QWaylandOutput::sendFrameCallbacks()
{
    Q_D(QWaylandOutput);
    for (int i = 0; i < d->surfaceViews.size(); i++) {
        const QWaylandSurfaceViewMapper &surfacemapper = d->surfaceViews.at(i);
        if (surfacemapper.surface && surfacemapper.surface->isMapped()) {
            if (!surfacemapper.has_entered) {
                surfaceEnter(surfacemapper.surface);
                d->surfaceViews[i].has_entered = true;
            }
            if (surfacemapper.maybeThrottelingView())
                surfacemapper.surface->sendFrameCallbacks();
        }
    }
    wl_display_flush_clients(d->compositor()->display());
}

void QWaylandOutput::surfaceEnter(QWaylandSurface *surface)
{
    if (!surface)
        return;
    QWaylandSurfacePrivate::get(surface)->send_enter(resourceForClient(surface->client()));
}

void QWaylandOutput::surfaceLeave(QWaylandSurface *surface)
{
    if (!surface || !surface->client())
        return;
    QWaylandSurfacePrivate::get(surface)->send_leave(resourceForClient(surface->client()));
}

void QWaylandOutput::setWidth(int newWidth)
{
    Q_D(QWaylandOutput);
    if (d->mode.size.width() == newWidth)
        return;

    QSize s = d->mode.size;
    s.setWidth(newWidth);
    setGeometry(QRect(d->position, s));
}

void QWaylandOutput::setHeight(int newHeight)
{
    Q_D(QWaylandOutput);
    if (d->mode.size.height() == newHeight)
        return;

    QSize s = d->mode.size;
    s.setHeight(newHeight);
    setGeometry(QRect(d->position, s));
}

QPointF QWaylandOutput::mapToOutputSpace(const QPointF &point)
{
    return point + d_func()->position;
}

void QWaylandOutput::handleWindowDestroyed()
{
    Q_D(QWaylandOutput);
    d->window = Q_NULLPTR;
    emit windowDestroyed();
}

bool QWaylandOutput::event(QEvent *event)
{
    if (event->type() == QEvent::Polish)
        initialize();
    return QObject::event(event);
}

QT_END_NAMESPACE
