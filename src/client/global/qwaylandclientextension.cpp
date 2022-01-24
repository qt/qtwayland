/****************************************************************************
**
** Copyright (C) 2017 Erik Larsson.
** Copyright (C) 2021 David Redondo <qt@david-redondo.de>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#include "qwaylandclientextension.h"
#include "qwaylandclientextension_p.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

using RegistryGlobal = QtWaylandClient::QWaylandDisplay::RegistryGlobal;

QWaylandClientExtensionPrivate::QWaylandClientExtensionPrivate()
{
    // Keep the possibility to use a custom waylandIntegration as a plugin,
    // but also add the possibility to run it as a QML component.
    waylandIntegration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    if (!waylandIntegration)
        waylandIntegration = new QtWaylandClient::QWaylandIntegration();

    if (!waylandIntegration->nativeInterface()->nativeResourceForIntegration("wl_display"))
        qWarning() << "This application requires a Wayland platform plugin";
}

void QWaylandClientExtensionPrivate::globalAdded(const RegistryGlobal &global)
{
    Q_Q(QWaylandClientExtension);
    if (!active && global.interface == QLatin1String(q->extensionInterface()->name)) {
        q->bind(global.registry, global.id, global.version);
        active = true;
        emit q->activeChanged();
    }
}

void QWaylandClientExtensionPrivate::globalRemoved(const RegistryGlobal &global)
{
    Q_Q(QWaylandClientExtension);
    if (active && global.interface == QLatin1String(q->extensionInterface()->name)) {
        active = false;
        emit q->activeChanged();
    }
}

void QWaylandClientExtension::initialize()
{
    Q_D(QWaylandClientExtension);
    if (d->active) {
        return;
    }
    const QtWaylandClient::QWaylandDisplay *display = d->waylandIntegration->display();
    const auto globals = display->globals();
    auto global =
            std::find_if(globals.cbegin(), globals.cend(), [this](const RegistryGlobal &global) {
                return global.interface == QLatin1String(extensionInterface()->name);
            });
    if (global != globals.cend()) {
        bind(global->registry, global->id, global->version);
        d->active = true;
        emit activeChanged();
    }
}

QWaylandClientExtension::QWaylandClientExtension(const int ver)
    : QObject(*new QWaylandClientExtensionPrivate())
{
    Q_D(QWaylandClientExtension);
    d->version = ver;
    auto display = d->waylandIntegration->display();
    QObjectPrivate::connect(display, &QtWaylandClient::QWaylandDisplay::globalAdded, d,
                            &QWaylandClientExtensionPrivate::globalAdded);
    QObjectPrivate::connect(display, &QtWaylandClient::QWaylandDisplay::globalRemoved, d,
                            &QWaylandClientExtensionPrivate::globalRemoved);
    // This function uses virtual functions and we don't want it to be called from the constructor.
    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

QWaylandClientExtension::~QWaylandClientExtension()
{
}

QtWaylandClient::QWaylandIntegration *QWaylandClientExtension::integration() const
{
    Q_D(const QWaylandClientExtension);
    return d->waylandIntegration;
}

int QWaylandClientExtension::version() const
{
    Q_D(const QWaylandClientExtension);
    return d->version;
}

void QWaylandClientExtension::setVersion(const int ver)
{
    Q_D(QWaylandClientExtension);
    if (d->version != ver) {
        d->version = ver;
        emit versionChanged();
    }
}

bool QWaylandClientExtension::isActive() const
{
    Q_D(const QWaylandClientExtension);
    return d->active;
}

QT_END_NAMESPACE
