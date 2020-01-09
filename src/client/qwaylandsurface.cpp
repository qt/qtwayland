/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
