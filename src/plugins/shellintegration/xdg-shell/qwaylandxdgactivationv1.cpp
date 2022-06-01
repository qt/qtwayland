/****************************************************************************
**
** Copyright (C) 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "qwaylandxdgactivationv1_p.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgActivationV1::QWaylandXdgActivationV1(wl_registry *registry, uint32_t id,
                                                 uint32_t availableVersion)
    : QtWayland::xdg_activation_v1(registry, id, qMin(availableVersion, 1u))
{
}

QWaylandXdgActivationV1::~QWaylandXdgActivationV1()
{
    Q_ASSERT(isInitialized());
    destroy();
}

QWaylandXdgActivationTokenV1 *
QWaylandXdgActivationV1::requestXdgActivationToken(QWaylandDisplay *display,
                                                   struct ::wl_surface *surface, uint32_t serial,
                                                   const QString &app_id)
{
    auto wl = get_activation_token();
    auto provider = new QWaylandXdgActivationTokenV1;
    provider->init(wl);

    if (surface)
        provider->set_surface(surface);

    if (!app_id.isEmpty())
        provider->set_app_id(app_id);

    if (display->lastInputDevice())
        provider->set_serial(serial, display->lastInputDevice()->wl_seat());
    provider->commit();
    return provider;
}

}

QT_END_NAMESPACE

#include "moc_qwaylandxdgactivationv1_p.cpp"
