// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCOMPOSITORQUICKEXTENSIONS_P_H
#define QWAYLANDCOMPOSITORQUICKEXTENSIONS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQml/qqml.h>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandquickextension.h>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/qwaylandquickcompositor.h>
#include <QtWaylandCompositor/qwaylandqtwindowmanager.h>
#include <QtWaylandCompositor/qwaylandtextinputmanager.h>
#include <QtCore/private/qglobal_p.h>
#include <QtWaylandCompositor/qwaylandtextinputmanagerv3.h>
#include <QtWaylandCompositor/qwaylandqttextinputmethodmanager.h>
#include <QtWaylandCompositor/qwaylandidleinhibitv1.h>

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickCompositorQuickExtensionContainer : public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QWaylandCompositorExtension> extensions READ extensions)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_CLASSINFO("DefaultProperty", "data")
    QML_NAMED_ELEMENT(WaylandCompositor)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QQmlListProperty<QObject> data()
    {
        return QQmlListProperty<QObject>(this, &m_objects);
    }

    QQmlListProperty<QWaylandCompositorExtension> extensions()
    {
        return QQmlListProperty<QWaylandCompositorExtension>(this, this,
                                                   &append_extension,
                                                   &countFunction,
                                                   &atFunction,
                                                   &clearFunction);
    }

    static qsizetype countFunction(QQmlListProperty<QWaylandCompositorExtension> *list)
    {
        return static_cast<QWaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.size();
    }

    static QWaylandCompositorExtension *atFunction(QQmlListProperty<QWaylandCompositorExtension> *list, qsizetype index)
    {
        return static_cast<QWaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.at(index);
    }

    static void append_extension(QQmlListProperty<QWaylandCompositorExtension> *list, QWaylandCompositorExtension *extension)
    {
        QWaylandQuickCompositorQuickExtensionContainer *quickExtObj = static_cast<QWaylandQuickCompositorQuickExtensionContainer *>(list->data);
        extension->setExtensionContainer(quickExtObj);
    }

    static void clearFunction(QQmlListProperty<QWaylandCompositorExtension> *list)
    {
        static_cast<QWaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.clear();
    }

private:
    QList<QObject *> m_objects;
};


// Note: These have to be in a header with a Q_OBJECT macro, otherwise we won't run moc on it
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandQtWindowManager, QtWindowManager, 1, 0)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandIdleInhibitManagerV1,
                                                   IdleInhibitManagerV1, 1, 0)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandTextInputManager, TextInputManager, 1, 0)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandTextInputManagerV3, TextInputManagerV3,
                                                   1, 0)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandQtTextInputMethodManager,
                                                   QtTextInputMethodManager, 1, 0)

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITORQUICKEXTENSIONS_P_H
