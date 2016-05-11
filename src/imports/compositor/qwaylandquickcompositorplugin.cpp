/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QtCore/QDir>

#include <QtQml/qqmlextensionplugin.h>

#include <QtQuick/QQuickItem>

#include <QtWaylandCompositor/QWaylandQuickCompositor>
#include <QtWaylandCompositor/QWaylandQuickItem>
#include <QtWaylandCompositor/QWaylandQuickSurface>
#include <QtWaylandCompositor/QWaylandClient>
#include <QtWaylandCompositor/QWaylandQuickOutput>
#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandInputDevice>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandResource>

#include <QtWaylandCompositor/QWaylandWindowManagerExtension>
#include <QtWaylandCompositor/QWaylandWlShell>
#include <QtWaylandCompositor/QWaylandTextInputManager>
#include <QtWaylandCompositor/QWaylandXdgShell>

#include <QtWaylandCompositor/qwaylandexport.h>
#include "qwaylandmousetracker_p.h"

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS(QWaylandQuickCompositor)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandWindowManagerExtension)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandWlShell)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandXdgShell)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandTextInputManager)

class QmlUrlResolver
{
public:
    QmlUrlResolver(bool useResource, const QDir &qmlDir, const QString &qrcPath)
        : m_useResource(useResource)
        , m_qmlDir(qmlDir)
        , m_qrcPath(qrcPath)
    { }

    QUrl get(const QString &fileName)
    {
        return m_useResource ? QUrl(m_qrcPath + fileName) :
            QUrl::fromLocalFile(m_qmlDir.filePath(fileName));
    }
private:
    bool m_useResource;
    const QDir m_qmlDir;
    const QString m_qrcPath;
};


/*!
    \qmlmodule QtWayland.Compositor 1.0
    \title Qt Wayland Compositor QML Types
    \ingroup qmlmodules
    \brief Provides QML types for creating Wayland compositors

    This QML module contains types for creating Wayland compositors.

    To use the types in this module, import the module with the following line:

    \code
    import QtWayland.Compositor 1.0
    \endcode
*/



//![class decl]
class QWaylandCompositorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    virtual void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor"));
        defineModule(uri);

        bool useResource = true;
        QDir qmlDir(baseUrl().toLocalFile());
        if (qmlDir.exists(QStringLiteral("WaylandCursorItem.qml")))
            useResource = false;

        QmlUrlResolver resolver(useResource, qmlDir, QStringLiteral("qrc:/QtWayland/Compositor/"));

        qmlRegisterType(resolver.get(QStringLiteral("WaylandOutputWindow.qml")), uri, 1, 0, "WaylandOutputWindow");
        qmlRegisterType(resolver.get(QStringLiteral("WaylandCursorItem.qml")), uri, 1, 0, "WaylandCursorItem");
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterType<QWaylandQuickCompositorQuickExtensionContainer>(uri, 1, 0, "WaylandCompositor");
        qmlRegisterType<QWaylandQuickItem>(uri, 1, 0, "WaylandQuickItem");
        qmlRegisterType<QWaylandMouseTracker>(uri, 1, 0, "WaylandMouseTracker");
        qmlRegisterType<QWaylandQuickOutput>(uri, 1, 0, "WaylandOutput");
        qmlRegisterType<QWaylandQuickSurface>(uri, 1, 0, "WaylandSurface");

        qmlRegisterUncreatableType<QWaylandCompositorExtension>(uri, 1, 0, "WaylandExtension", QObject::tr("Cannot create instance of WaylandExtension"));
        qmlRegisterUncreatableType<QWaylandClient>(uri, 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
        qmlRegisterUncreatableType<QWaylandView>(uri, 1, 0, "WaylandView", QObject::tr("Cannot create instance of WaylandView, it can be retrieved by accessor on WaylandQuickItem"));
        qmlRegisterUncreatableType<QWaylandInputDevice>(uri, 1, 0, "WaylandInputDevice", QObject::tr("Cannot create instance of WaylandInputDevice"));
        qmlRegisterUncreatableType<QWaylandCompositor>(uri, 1, 0, "WaylandCompositorBase", QObject::tr("Cannot create instance of WaylandCompositorBase, use WaylandCompositor instead"));
        qmlRegisterUncreatableType<QWaylandSurface>(uri, 1, 0, "WaylandSurfaceBase", QObject::tr("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead"));
        qmlRegisterUncreatableType<QWaylandShellSurface>(uri, 1, 0, "ShellSurface", QObject::tr("Cannot create instance of ShellSurface"));
        qmlRegisterUncreatableType<QWaylandWlShellSurface>(uri, 1, 0, "WlShellSurfaceBase", QObject::tr("Cannot create instance of WlShellSurfaceBase, use WlShellSurface instead"));
        qmlRegisterUncreatableType<QWaylandXdgSurface>(uri, 1, 0, "XdgSurfaceBase", QObject::tr("Cannot create instance of XdgSurfaceBase, use XdgSurface instead"));
        qmlRegisterUncreatableType<QWaylandResource>(uri, 1, 0, "WaylandResource", QObject::tr("Cannot create instance of WaylandResource"));

        //This should probably be somewhere else
        qmlRegisterType<QWaylandWindowManagerExtensionQuickExtension>(uri, 1, 0, "WindowManager");
        qmlRegisterType<QWaylandWlShellQuickExtension>(uri, 1, 0, "WlShell");
        qmlRegisterType<QWaylandWlShellSurface>(uri, 1, 0, "WlShellSurface");
        qmlRegisterType<QWaylandQuickShellSurfaceItem>(uri, 1, 0, "ShellSurfaceItem");
        qmlRegisterType<QWaylandXdgShellQuickExtension>(uri, 1, 0, "XdgShell");
        qmlRegisterType<QWaylandXdgSurface>(uri, 1, 0, "XdgSurface");
        qmlRegisterType<QWaylandTextInputManagerQuickExtension>(uri, 1, 0, "TextInputManager");
    }
};
//![class decl]

QT_END_NAMESPACE

#include "qwaylandquickcompositorplugin.moc"
