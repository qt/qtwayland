/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwloutput_p.h"
#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"

#include <QtGui/QWindow>
#include <QRect>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandOutput>

#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandSurfaceView>

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

Output::Output(QWaylandOutput *output, QWaylandOutputSpace *outputSpace, QWindow *window)
    : QtWaylandServer::wl_output(outputSpace->compositor()->waylandDisplay(), 2)
    , m_window(window)
    , m_output(output)
    , m_outputSpace(Q_NULLPTR)
    , m_position(QPoint())
    , m_availableGeometry(QRect())
    , m_physicalSize(QSize())
    , m_subpixel(QWaylandOutput::SubpixelUnknown)
    , m_transform(QWaylandOutput::TransformNormal)
    , m_scaleFactor(1)
    , m_sizeFollowsWindow(true)
{
    m_mode.size = window ? window->size() : QSize();
    m_mode.refreshRate = 60;
    setOutputSpace(outputSpace, false);

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

bool Output::sizeFollowsWindow() const
{
    return m_sizeFollowsWindow;
}

void Output::setSizeFollowsWindow(bool follow)
{
    if (follow != m_sizeFollowsWindow) {
        if (follow) {
            QObject::connect(m_window, &QWindow::widthChanged, m_output, &QWaylandOutput::setWidth);
            QObject::connect(m_window, &QWindow::heightChanged, m_output, &QWaylandOutput::setHeight);
        } else {
            QObject::disconnect(m_window, &QWindow::widthChanged, m_output, &QWaylandOutput::setWidth);
            QObject::disconnect(m_window, &QWindow::heightChanged, m_output, &QWaylandOutput::setHeight);
        }
        m_sizeFollowsWindow = follow;
        m_output->sizeFollowsWindowChanged();
    }
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

void Output::setWidth(int newWidth)
{
    if (m_mode.size.width() == newWidth)
        return;

    QSize s = m_mode.size;
    s.setWidth(newWidth);
    setGeometry(QRect(m_position, s));
}

void Output::setHeight(int newHeight)
{
    if (m_mode.size.height() == newHeight)
        return;

    QSize s = m_mode.size;
    s.setHeight(newHeight);
    setGeometry(QRect(m_position, s));
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

void Output::setOutputSpace(QWaylandOutputSpace *outputSpace, bool setOutputSpace)
{
    if (m_outputSpace == outputSpace)
        return;

    if (m_outputSpace) {
        m_outputSpace->removeOutput(waylandOutput());
    }

    m_outputSpace = outputSpace;

    if (outputSpace && setOutputSpace) {
        outputSpace->addOutput(waylandOutput());
    }

    waylandOutput()->outputSpaceChanged();
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


void Output::frameStarted()
{
    for (int i = 0; i < m_surfaceViews.size(); i++) {
        SurfaceViewMapper &surfacemapper = m_surfaceViews[i];
        if (surfacemapper.surface && (!surfacemapper.surface->primaryOutput()
                                      || surfacemapper.surface->primaryOutput()->handle() == this))
            surfacemapper.surface->handle()->frameStarted();
    }
}

void Output::sendFrameCallbacks()
{
    for (int i = 0; i < m_surfaceViews.size(); i++) {
        const SurfaceViewMapper &surfacemapper = m_surfaceViews.at(i);
        if (surfacemapper.surface && surfacemapper.surface->isMapped()
            && (!surfacemapper.surface->primaryOutput()
                || surfacemapper.surface->primaryOutput()->handle() == this))
            surfacemapper.surface->handle()->sendFrameCallback();
    }
    wl_display_flush_clients(compositor()->waylandDisplay());
}

void Output::addView(QWaylandSurfaceView *view)
{
    addView(view, view->surface());
}

void Output::addView(QWaylandSurfaceView *view, QWaylandSurface *surface)
{
    for (int i = 0; i < m_surfaceViews.size(); i++) {
        if (surface == m_surfaceViews.at(i).surface) {
            if (!m_surfaceViews.at(i).views.contains(view)) {
                m_surfaceViews[i].views.append(view);
            }
            return;
        }
    }

    SurfaceViewMapper surfaceViewMapper;
    surfaceViewMapper.surface = surface;
    surfaceViewMapper.views.append(view);
    m_surfaceViews.append(surfaceViewMapper);
    if (surface)
        surface->enter(waylandOutput());
}

void Output::removeView(QWaylandSurfaceView *view)
{
    removeView(view, view->surface());
}

void Output::removeView(QWaylandSurfaceView *view, QWaylandSurface *surface)
{
    for (int i = 0; i < m_surfaceViews.size(); i++) {
        if (surface == m_surfaceViews.at(i).surface) {
            bool removed = m_surfaceViews[i].views.removeOne(view);
            if (m_surfaceViews.at(i).views.isEmpty() && removed) {
                m_surfaceViews.remove(i);
                if (surface)
                    surface->leave(waylandOutput());
            }
            return;
        }
    }
    qWarning("%s Could not find view %p for surface %p to remove. Possible invalid state", Q_FUNC_INFO, view, surface);
}

void Output::updateSurfaceForView(QWaylandSurfaceView *view, QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    removeView(view, oldSurface);
    addView(view, newSurface);
}

} // namespace Wayland

QT_END_NAMESPACE
