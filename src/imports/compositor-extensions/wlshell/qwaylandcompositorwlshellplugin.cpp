// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include <QtWaylandCompositor/qwaylandwlshell.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandWlShell)

/*!
    \qmlmodule QtWayland.Compositor.WlShell
    \title Qt Wayland WlShell extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the WlShell extension.

    \section2 Summary
    WlShell is a shell extension providing window system features typical to
    desktop systems. It is superseded by XdgShell and exists in Qt mainly
    for backwards compatibility with older applications.

    WlShell corresponds to the Wayland interface \c wl_shell.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.WlShell
    \endqml
*/

class QWaylandCompositorWlShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor.WlShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
        qmlRegisterType<QWaylandWlShellQuickExtension>(uri, 1, 0, "WlShell");
        qmlRegisterType<QWaylandWlShellSurface>(uri, 1, 0, "WlShellSurface");
    }
};

QT_END_NAMESPACE

#include "qwaylandcompositorwlshellplugin.moc"
