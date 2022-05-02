/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
******************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include <QtWaylandCompositor/qwaylandiviapplication.h>
#include <QtWaylandCompositor/qwaylandivisurface.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandIviApplication)

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
