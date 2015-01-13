/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include <QtCompositor/QWaylandQuickCompositor>
#include <QtCompositor/QWaylandSurfaceItem>
#include <QtCompositor/QWaylandQuickSurface>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandOutput>

#include <QtCompositor/private/qwlcompositor_p.h>

QT_BEGIN_NAMESPACE

class QWaylandQuickCompositorImpl : public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    QWaylandQuickCompositorImpl(QObject *parent = 0)
        : QWaylandQuickCompositor(parent)
    {
        setInitializeLegazyQmlNames(false);
    }

    QQmlListProperty<QObject> data()
    {
        return QQmlListProperty<QObject>(this, m_objects);
    }

protected:
    void classBegin() Q_DECL_OVERRIDE
    {
        QWaylandQuickCompositor::classBegin();
    }

    void componentComplete() Q_DECL_OVERRIDE
    {
        create();
        QWaylandQuickCompositor::componentComplete();
    }
private:
    QList<QObject *> m_objects;
};

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
    const QDir &m_qmlDir;
    const QString &m_qrcPath;
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
        if (qmlDir.exists(QStringLiteral("WaylandSurfaceChrome.qml")))
            useResource = false;

        QmlUrlResolver resolver(useResource, qmlDir, QStringLiteral("qrc:/QtWayland/Compositor/"));

        qmlRegisterType(resolver.get(QStringLiteral("WaylandSurfaceChrome.qml")), uri, 1, 0, "WaylandSurfaceChrome");
        qmlRegisterType(resolver.get(QStringLiteral("WaylandOutputWindow.qml")), uri, 1, 0, "WaylandOutputWindow");
        qmlRegisterType(resolver.get(QStringLiteral("WaylandCursorItem.qml")), uri, 1, 0, "WaylandCursorItem");
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterType<QWaylandQuickCompositorImpl>(uri, 1, 0, "WaylandCompositor");
        qmlRegisterType<QWaylandSurfaceItem>(uri, 1, 0, "WaylandSurfaceView");
        qmlRegisterUncreatableType<QWaylandQuickSurface>(uri, 1, 0, "WaylandQuickSurface", QObject::tr("Cannot create instance of WaylandQuickSurface"));
        qmlRegisterUncreatableType<QWaylandClient>(uri, 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
        qmlRegisterUncreatableType<QWaylandOutput>(uri, 1, 0, "WaylandOutput", QObject::tr("Cannot create instance of WaylandOutput"));
    }
};
//![class decl]

QT_END_NAMESPACE

#include "qwaylandquickcompositorplugin.moc"
