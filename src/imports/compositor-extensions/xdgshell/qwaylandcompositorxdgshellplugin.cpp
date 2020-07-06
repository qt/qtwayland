/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandXdgDecorationManagerV1>
#include <QtWaylandCompositor/QWaylandQuickXdgOutputV1>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandXdgShell)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandXdgDecorationManagerV1)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandXdgOutputManagerV1)

class QWaylandCompositorXdgShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor.XdgShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);

        qmlRegisterType<QWaylandXdgShellQuickExtension>(uri, 1, 3, "XdgShell");
        qmlRegisterType<QWaylandXdgSurface>(uri, 1, 3, "XdgSurface");
        qmlRegisterUncreatableType<QWaylandXdgToplevel>(uri, 1, 3, "XdgToplevel", QObject::tr("Cannot create instance of XdgShellToplevel"));
        qmlRegisterUncreatableType<QWaylandXdgPopup>(uri, 1, 3, "XdgPopup", QObject::tr("Cannot create instance of XdgShellPopup"));

        qmlRegisterType<QWaylandXdgDecorationManagerV1QuickExtension>(uri, 1, 3, "XdgDecorationManagerV1");
        qmlRegisterType<QWaylandXdgOutputManagerV1QuickExtension>(uri, 1, 14, "XdgOutputManagerV1");
        qmlRegisterType<QWaylandQuickXdgOutputV1>(uri, 1, 14, "XdgOutputV1");
    }
};

QT_END_NAMESPACE

#include "qwaylandcompositorxdgshellplugin.moc"
