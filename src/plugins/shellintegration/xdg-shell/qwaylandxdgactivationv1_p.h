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

#ifndef QWAYLANDXDGACTIVATIONV1_P_H
#define QWAYLANDXDGACTIVATIONV1_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include "qwayland-xdg-activation-v1.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgActivationTokenV1
    : public QObject,
      public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    void xdg_activation_token_v1_done(const QString &token) override { Q_EMIT done(token); }

Q_SIGNALS:
    void done(const QString &token);
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgActivationV1 : public QtWayland::xdg_activation_v1
{
public:
    QWaylandXdgActivationV1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~QWaylandXdgActivationV1() override;

    QWaylandXdgActivationTokenV1 *requestXdgActivationToken(QWaylandDisplay *display,
                                                            struct ::wl_surface *surface,
                                                            uint32_t serial, const QString &app_id);
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGACTIVATIONV1_P_H
