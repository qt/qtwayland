/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qwaylandsurface_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandscreen_p.h"

#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandSurface::QWaylandSurface(QWaylandDisplay *display)
    : wl_surface(display->createSurface(this))
{
    connect(qApp, &QGuiApplication::screenRemoved, this, &QWaylandSurface::handleScreenRemoved);
}

QWaylandSurface::~QWaylandSurface()
{
    destroy();
}

QWaylandScreen *QWaylandSurface::oldestEnteredScreen()
{
    return m_screens.value(0, nullptr);
}

QWaylandSurface *QWaylandSurface::fromWlSurface(::wl_surface *surface)
{
    if (auto *s = QtWayland::wl_surface::fromObject(surface))
        return static_cast<QWaylandSurface *>(s);
    return nullptr;
}

void QWaylandSurface::handleScreenRemoved(QScreen *qScreen)
{
    auto *platformScreen = qScreen->handle();
    if (platformScreen->isPlaceholder())
        return;

    auto *waylandScreen = static_cast<QWaylandScreen *>(qScreen->handle());
    if (m_screens.removeOne(waylandScreen))
        emit screensChanged();
}

void QWaylandSurface::surface_enter(wl_output *output)
{
    auto addedScreen = QWaylandScreen::fromWlOutput(output);

    if (!addedScreen)
        return;

    if (m_screens.contains(addedScreen)) {
        qCWarning(lcQpaWayland)
                << "Ignoring unexpected wl_surface.enter received for output with id:"
                << wl_proxy_get_id(reinterpret_cast<wl_proxy *>(output))
                << "screen name:" << addedScreen->name() << "screen model:" << addedScreen->model()
                << "This is most likely a bug in the compositor.";
        return;
    }

    m_screens.append(addedScreen);
    emit screensChanged();
}

void QWaylandSurface::surface_leave(wl_output *output)
{
    auto *removedScreen = QWaylandScreen::fromWlOutput(output);

    if (!removedScreen)
        return;

    bool wasRemoved = m_screens.removeOne(removedScreen);
    if (!wasRemoved) {
        qCWarning(lcQpaWayland)
                << "Ignoring unexpected wl_surface.leave received for output with id:"
                << wl_proxy_get_id(reinterpret_cast<wl_proxy *>(output))
                << "screen name:" << removedScreen->name()
                << "screen model:" << removedScreen->model()
                << "This is most likely a bug in the compositor.";
        return;
    }
    emit screensChanged();
}

} // namespace QtWaylandClient

QT_END_NAMESPACE

#include "moc_qwaylandsurface_p.cpp"
