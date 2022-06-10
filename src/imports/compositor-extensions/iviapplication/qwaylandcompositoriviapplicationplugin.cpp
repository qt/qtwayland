// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include <QtWaylandCompositor/qwaylandiviapplication.h>
#include <QtWaylandCompositor/qwaylandivisurface.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandIviApplication)

/*!
    \qmlmodule QtWayland.Compositor.IviApplication
    \title Qt Wayland IviApplication Extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the IviApplication shell extension.

    \section2 Summary
    IviApplication is a shell extension suitable for lightweight compositors,
    for example in In-Vehicle Infotainment (IVI) systems.

    IviApplication corresponds to the Wayland \c ivi_application interface.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.IviApplication
    \endqml
*/

class QWaylandCompositorIviApplicationPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor.IviApplication"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
        qmlRegisterType<QWaylandIviApplicationQuickExtension>(uri, 1, 0, "IviApplication");
        qmlRegisterType<QWaylandIviSurface>(uri, 1, 0, "IviSurface");
    }
};

QT_END_NAMESPACE

#include "qwaylandcompositoriviapplicationplugin.moc"
