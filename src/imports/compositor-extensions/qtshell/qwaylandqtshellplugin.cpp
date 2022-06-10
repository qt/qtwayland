// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include "qwaylandqtshell.h"
#include "qwaylandqtshellchrome.h"

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandQtShell)

/*!
    \qmlmodule QtWayland.Compositor.QtShell
    \title Qt Wayland Qt Shell Extension
    \ingroup qmlmodules
    \since 6.3
    \brief Provides a shell extension for Qt applications running on a Qt Wayland Compositor.

    \section2 Summary
    The QtShell extension provides a way to associate an QtShellSurface with a regular Wayland
    surface. The QtShell extension is written to support the window management features which are
    supported by Qt. It may be suitable on a platform where both the compositor and client
    applications are written with Qt, and where applications are trusted not to abuse features such
    as manual window positioning and "bring-to-front".

    For other use cases, consider using IviApplication or XdgShell instead.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.IviApplication
    \endqml
*/

class QQtWaylandShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor.QtShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
        qmlRegisterType<QWaylandQtShellQuickExtension>(uri, 1, 0, "QtShell");
        qmlRegisterType<QWaylandQtShellSurface>(uri, 1, 0, "QtShellSurface");
        qmlRegisterType<QWaylandQtShellChrome>(uri, 1, 0, "QtShellChrome");
    }
};

QT_END_NAMESPACE

#include "qwaylandqtshellplugin.moc"
