/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqmlengine.h>

#include "sharedtextureprovider_p.h"

/*!
    \internal
    \qmlmodule QtWayland.Client.TextureSharing 1
    \title Qt Wayland Shared Texture Provider
    \ingroup qmlmodules
    \brief Adds an image provider which utilizes shared GPU memory

    \section2 Summary

    This module allows Qt Wayland clients to use graphical resources exported
    by the compositor, without allocating any graphics memory in the client.

    \note The texture sharing functionality is considered experimental and
    currently unsupported in Qt 6.

    \section2 Usage

    To use this module, import it like this:
    \code
    import QtWayland.Client.TextureSharing 1.0
    \endcode

    The sharing functionality is provided through a QQuickImageProvider. Use
    the "image:" scheme for the URL source of the image, followed by the
    identifier \e wlshared, followed by the image file path. For example:

    \code
    Image { source: "image://wlshared/wallpapers/mybackground.jpg" }
    \endcode

    The shared texture module does not provide any directly usable QML types.
*/

QT_BEGIN_NAMESPACE

class QWaylandTextureSharingPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QWaylandTextureSharingPlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) {}

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(uri == QStringLiteral("QtWayland.Client.TextureSharing"));
        qmlRegisterModule(uri, 1, 0);
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_UNUSED(uri);
        engine->addImageProvider("wlshared", new SharedTextureProvider);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
