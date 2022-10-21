/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
#include <QtWaylandCompositor/qwaylandqttextinputmethodmanager.h>
#include <QtWaylandCompositor/qwaylandidleinhibitv1.h>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickCompositorQuickExtensionContainer : public QWaylandQuickCompositor
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
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(QWaylandQtWindowManager, QtWindowManager)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(QWaylandIdleInhibitManagerV1, IdleInhibitManagerV1)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(QWaylandTextInputManager, TextInputManager)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(QWaylandQtTextInputMethodManager, QtTextInputMethodManager)

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITORQUICKEXTENSIONS_P_H
