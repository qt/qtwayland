/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>
#include <QtGui/QWindow>
#include <QtGui/QExposeEvent>
#include <private/qobject_p.h>

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwloutput_p.h"
#include "qwaylandcompositor.h"
#include "qwaylandoutput.h"

QWaylandOutput::QWaylandOutput(QWaylandCompositor *compositor, QWindow *window,
                               const QString &manufacturer, const QString &model)
    : QObject()
    , d_ptr(new QtWayland::Output(compositor->handle(), window))
{
    d_ptr->m_output = this;
    d_ptr->setManufacturer(manufacturer);
    d_ptr->setModel(model);
    d_ptr->compositor()->addOutput(this);
}

QWaylandOutput::~QWaylandOutput()
{
    d_ptr->compositor()->removeOutput(this);
    delete d_ptr;
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

    return output->output();
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
    return d_ptr->compositor()->waylandCompositor();
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

QWindow *QWaylandOutput::window() const
{
    return d_ptr->window();
}

QtWayland::Output *QWaylandOutput::handle()
{
    return d_ptr;
}
