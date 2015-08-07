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

#include "qwaylandcompositor.h"
#include "qwaylandview.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>
#include <QtGui/QWindow>
#include <QtGui/QExposeEvent>
#include <private/qobject_p.h>

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwloutput_p.h"

QT_BEGIN_NAMESPACE

QWaylandOutput::QWaylandOutput(QWaylandOutputSpace *outputSpace, QWindow *window,
                               const QString &manufacturer, const QString &model)
    : QObject()
    , d_ptr(new QtWayland::Output(this, outputSpace, window))
{
    d_ptr->setManufacturer(manufacturer);
    d_ptr->setModel(model);
    QObject::connect(window, &QWindow::widthChanged, this, &QWaylandOutput::setWidth);
    QObject::connect(window, &QWindow::heightChanged, this, &QWaylandOutput::setHeight);
    QObject::connect(window, &QObject::destroyed, this, &QWaylandOutput::windowDestroyed);
}

QWaylandOutput::~QWaylandOutput()
{
    d_ptr->outputSpace()->removeOutput(this);
}

QWaylandOutput *QWaylandOutput::fromResource(wl_resource *resource)
{
    QtWayland::OutputResource *outputResource = static_cast<QtWayland::OutputResource *>(
        QtWayland::Output::Resource::fromResource(resource));
    if (!outputResource)
        return Q_NULLPTR;

    QtWayland::Output *output = static_cast<QtWayland::Output *>(outputResource->output_object);
    if (!output)
        return Q_NULLPTR;

    return output->waylandOutput();
}

void QWaylandOutput::setOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_ASSERT(outputSpace);
    d_ptr->setOutputSpace(outputSpace, true);
}

QWaylandOutputSpace *QWaylandOutput::outputSpace() const
{
    return d_ptr->outputSpace();
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
    return d_ptr->outputSpace()->compositor();
}

QString QWaylandOutput::manufacturer() const
{
    return d_ptr->manufacturer();
}

QString QWaylandOutput::model() const
{
    return d_ptr->model();
}

QPoint QWaylandOutput::position() const
{
    return d_ptr->position();
}

void QWaylandOutput::setPosition(const QPoint &pt)
{
    if (d_ptr->position() == pt)
        return;

    d_ptr->setPosition(pt);
    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();
}

QWaylandOutput::Mode QWaylandOutput::mode() const
{
    return d_ptr->mode();
}

void QWaylandOutput::setMode(const Mode &mode)
{
    if (d_ptr->mode().size == mode.size && d_ptr->mode().refreshRate == mode.refreshRate)
        return;

    d_ptr->setMode(mode);
    Q_EMIT modeChanged();
    Q_EMIT geometryChanged();

    if (window()) {
        window()->resize(mode.size);
        window()->setMinimumSize(mode.size);
        window()->setMaximumSize(mode.size);
    }
}

QRect QWaylandOutput::geometry() const
{
    return d_ptr->geometry();
}

void QWaylandOutput::setGeometry(const QRect &geometry)
{
    if (d_ptr->geometry() == geometry)
        return;

    d_ptr->setGeometry(geometry);
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
    if (!d_ptr->availableGeometry().isValid())
        return QRect(d_ptr->position(), d_ptr->mode().size);

    return d_ptr->availableGeometry();
}

void QWaylandOutput::setAvailableGeometry(const QRect &availableGeometry)
{
    if (d_ptr->availableGeometry() == availableGeometry)
        return;

    d_ptr->setAvailableGeometry(availableGeometry);
    Q_EMIT availableGeometryChanged();
}

QSize QWaylandOutput::physicalSize() const
{
    return d_ptr->physicalSize();
}

void QWaylandOutput::setPhysicalSize(const QSize &size)
{
    if (d_ptr->physicalSize() == size)
        return;

    d_ptr->setPhysicalSize(size);
    Q_EMIT physicalSizeChanged();
}

QWaylandOutput::Subpixel QWaylandOutput::subpixel() const
{
    return d_ptr->subpixel();
}

void QWaylandOutput::setSubpixel(const Subpixel &subpixel)
{
    if (d_ptr->subpixel() == subpixel)
        return;

    d_ptr->setSubpixel(subpixel);
    Q_EMIT subpixelChanged();
}

QWaylandOutput::Transform QWaylandOutput::transform() const
{
    return d_ptr->transform();
}

void QWaylandOutput::setTransform(const Transform &transform)
{
    if (d_ptr->transform() == transform)
        return;

    d_ptr->setTransform(transform);
    Q_EMIT transformChanged();
}

int QWaylandOutput::scaleFactor() const
{
    return d_ptr->scaleFactor();
}

void QWaylandOutput::setScaleFactor(int scale)
{
    if (d_ptr->scaleFactor() == scale)
        return;

    d_ptr->setScaleFactor(scale);
    Q_EMIT scaleFactorChanged();

}

bool QWaylandOutput::sizeFollowsWindow() const
{
    return d_ptr->sizeFollowsWindow();
}

void QWaylandOutput::setSizeFollowsWindow(bool follow)
{
    d_ptr->setSizeFollowsWindow(follow);
}

QWindow *QWaylandOutput::window() const
{
    return d_ptr->window();
}

void QWaylandOutput::frameStarted()
{
    d_ptr->frameStarted();
}

void QWaylandOutput::sendFrameCallbacks()
{
    d_ptr->sendFrameCallbacks();
}

void QWaylandOutput::surfaceEnter(QWaylandSurface *surface)
{
    d_ptr->surfaceEnter(surface);
}

void QWaylandOutput::surfaceLeave(QWaylandSurface *surface)
{
    d_ptr->surfaceLeave(surface);
}

QtWayland::Output *QWaylandOutput::handle() const
{
    return d_ptr.data();
}

QWaylandView *QWaylandOutput::pickView(const QPointF &outputPosition) const
{
    const QVector<QtWayland::SurfaceViewMapper> surfaceViewMappers = d_ptr->surfaceMappers();
    for (int nSurface = 0; surfaceViewMappers.size(); nSurface++) {
        const QWaylandSurface *surface = surfaceViewMappers.at(nSurface).surface;
        if (surface->isCursorSurface())
            continue;
        const QVector<QWaylandView *> views = surfaceViewMappers.at(nSurface).views;
        for (int nView = 0; views.size(); nView++) {
            if (QRectF(views.at(nView)->requestedPosition(), surface->size()).contains(outputPosition))
                return views.at(nView);
        }
    }
    return Q_NULLPTR;
}

QPointF QWaylandOutput::mapToView(QWaylandView *view, const QPointF &outputPosition) const
{
    return outputPosition - view->requestedPosition();
}

void QWaylandOutput::setWidth(int newWidth)
{
    d_ptr->setWidth(newWidth);
}

void QWaylandOutput::setHeight(int newHeight)
{
    d_ptr->setHeight(newHeight);
}

QPointF QWaylandOutput::mapToOutputSpace(const QPointF &point)
{
    return point + d_ptr->topLeft();
}

void QWaylandOutput::windowDestroyed()
{
    delete this;
}

QT_END_NAMESPACE
