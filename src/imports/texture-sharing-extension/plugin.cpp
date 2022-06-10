// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqmlengine.h>

#include "QtWaylandCompositor/private/qwltexturesharingextension_p.h"

/*!
    \internal
    \qmlmodule QtWayland.Compositor.TextureSharingExtension 1
    \title Qt Wayland Shared Texture Provider
    \ingroup qmlmodules
    \brief Adds a mechanism to share GPU memory

    \section2 Summary

    This module lets the compositor export graphical resources that can be used by clients,
    without allocating any graphics memory in the client.

    \note The texture sharing functionality is considered experimental and
    currently unsupported in Qt 6.

    \section2 Usage

    This module is imported like this:

    \code
    import QtWayland.Compositor.TextureSharingExtension 1.0
    \endcode

    To use this module in a compositor, instantiate the extension object as a child of the compositor object, like this:


    \code
    WaylandCompositor {
        //...
        TextureSharingExtension {
        }
    }
    \endcode

    The sharing functionality is provided through a QQuickImageProvider. Use
    the "image:" scheme for the URL source of the image, followed by the
    identifier \e wlshared, followed by the image file path. For example:

    \code
    Image { source: "image://wlshared/wallpapers/mybackground.jpg" }
    \endcode

*/

QT_BEGIN_NAMESPACE

class QWaylandTextureSharingExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QWaylandTextureSharingExtensionPlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) {}

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(uri == QStringLiteral("QtWayland.Compositor.TextureSharingExtension"));
        qmlRegisterType<QWaylandTextureSharingExtensionQuickExtension>("QtWayland.Compositor.TextureSharingExtension", 1, 0, "TextureSharingExtension");
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_UNUSED(uri);
        engine->addImageProvider("wlshared", new QWaylandSharedTextureProvider);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
