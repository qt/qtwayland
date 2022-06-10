// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

/*!
    \qmlmodule QtWayland.Compositor.XdgShell
    \title Qt Wayland XdgShell Extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the XdgShell shell extension.

    \section2 Summary
    XdgShell is a shell extension providing window system features typical to
    desktop systems.

    XdgShell corresponds to the Wayland interface, \c xdg_shell.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.XdgShell
    \endqml
*/

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
