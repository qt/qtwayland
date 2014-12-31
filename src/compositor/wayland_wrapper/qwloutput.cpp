/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwloutput_p.h"

#include "qwlcompositor_p.h"
#include "qwlextendedoutput_p.h"
#include "qwlsurface_p.h"

#include <QtGui/QWindow>
#include <QRect>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandOutput>

QT_BEGIN_NAMESPACE

namespace QtWayland {

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

Output::Output(Compositor *compositor, QWindow *window)
    : QtWaylandServer::wl_output(compositor->wl_display(), 2)
    , m_compositor(compositor)
    , m_window(window)
    , m_output(Q_NULLPTR)
    , m_position(QPoint())
    , m_availableGeometry(QRect())
    , m_physicalSize(QSize())
    , m_subpixel(QWaylandOutput::SubpixelUnknown)
    , m_transform(QWaylandOutput::TransformNormal)
    , m_scaleFactor(1)
{
    m_mode.size = window ? window->size() : QSize();
    m_mode.refreshRate = 60;

    qRegisterMetaType<QWaylandOutput::Mode>("WaylandOutput::Mode");
}

void Output::output_bind_resource(Resource *resource)
{
    send_geometry(resource->handle,
                  m_position.x(), m_position.y(),
                  m_physicalSize.width(), m_physicalSize.height(),
                  toWlSubpixel(m_subpixel), m_manufacturer, m_model,
                  toWlTransform(m_transform));

    send_mode(resource->handle, mode_current | mode_preferred,
              m_mode.size.width(), m_mode.size.height(),
              m_mode.refreshRate);

    if (resource->version() >= 2) {
        send_scale(resource->handle, m_scaleFactor);
        send_done(resource->handle);
    }
}

void Output::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

void Output::setModel(const QString &model)
{
    m_model = model;
}

void Output::setPosition(const QPoint &position)
{
    if (m_position == position)
        return;

    m_position = position;

    sendGeometryInfo();
}

void Output::setMode(const QWaylandOutput::Mode &mode)
{
    if (m_mode.size == mode.size && m_mode.refreshRate == mode.refreshRate)
        return;

    m_mode = mode;

    Q_FOREACH (Resource *resource, resourceMap().values()) {
        send_mode(resource->handle, mode_current,
                  m_mode.size.width(), m_mode.size.height(),
                  m_mode.refreshRate * 1000);
        if (resource->version() >= 2)
            send_done(resource->handle);
    }
}

QRect Output::geometry() const
{
    return QRect(m_position, m_mode.size);
}

void Output::setGeometry(const QRect &geometry)
{
    if (m_position == geometry.topLeft() && m_mode.size == geometry.size())
        return;

    m_position = geometry.topLeft();
    m_mode.size = geometry.size();

    Q_FOREACH (Resource *resource, resourceMap().values()) {
        send_geometry(resource->handle,
                      m_position.x(), m_position.y(),
                      m_physicalSize.width(), m_physicalSize.height(),
                      toWlSubpixel(m_subpixel), m_manufacturer, m_model,
                      toWlTransform(m_transform));
        send_mode(resource->handle, mode_current,
                  m_mode.size.width(), m_mode.size.height(),
                  m_mode.refreshRate * 1000);
        if (resource->version() >= 2)
            send_done(resource->handle);
    }
}

void Output::setAvailableGeometry(const QRect &availableGeometry)
{
    m_availableGeometry = availableGeometry;
}

void Output::setPhysicalSize(const QSize &physicalSize)
{
    if (m_physicalSize == physicalSize)
        return;

    m_physicalSize = physicalSize;

    sendGeometryInfo();
}

void Output::setSubpixel(const QWaylandOutput::Subpixel &subpixel)
{
    if (m_subpixel == subpixel)
        return;

    m_subpixel = subpixel;

    sendGeometryInfo();
}

void Output::setTransform(const QWaylandOutput::Transform &transform)
{
    if (m_transform == transform)
        return;

    m_transform = transform;

    sendGeometryInfo();
}

void Output::setScaleFactor(int scale)
{
    if (m_scaleFactor == scale)
        return;

    m_scaleFactor = scale;

    Q_FOREACH (Resource *resource, resourceMap().values()) {
        if (resource->version() >= 2) {
            send_scale(resource->handle, m_scaleFactor);
            send_done(resource->handle);
        }
    }
}

OutputResource *Output::outputForClient(wl_client *client) const
{
    return static_cast<OutputResource *>(resourceMap().value(client));
}

void Output::sendGeometryInfo()
{
    Q_FOREACH (Resource *resource, resourceMap().values()) {
        send_geometry(resource->handle,
                      m_position.x(), m_position.x(),
                      m_physicalSize.width(), m_physicalSize.height(),
                      toWlSubpixel(m_subpixel), m_manufacturer, m_model,
                      toWlTransform(m_transform));
        if (resource->version() >= 2)
            send_done(resource->handle);
    }
}

} // namespace Wayland

QT_END_NAMESPACE
