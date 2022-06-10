// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKCHILDREN_H
#define QWAYLANDQUICKCHILDREN_H

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

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#if QT_CONFIG(wayland_compositor_quick)
#include <QtQml/QQmlListProperty>
#include <QtCore/QList>
#endif

QT_BEGIN_NAMESPACE

#if QT_CONFIG(wayland_compositor_quick)
#define Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(className) \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, this, \
                                             &className::appendFunction, \
                                             &className::countFunction, \
                                             &className::atFunction, \
                                             &className::clearFunction); \
        } \
        static void appendFunction(QQmlListProperty<QObject> *list, QObject *object) \
        { \
            static_cast<className *>(list->data)->m_children.append(object); \
        } \
        static qsizetype countFunction(QQmlListProperty<QObject> *list) \
        { \
            return static_cast<className *>(list->data)->m_children.size(); \
        } \
        static QObject *atFunction(QQmlListProperty<QObject> *list, qsizetype index) \
        { \
            return static_cast<className *>(list->data)->m_children.at(index); \
        } \
        static void clearFunction(QQmlListProperty<QObject> *list) \
        { \
            static_cast<className *>(list->data)->m_children.clear(); \
        } \
    private: \
        QList<QObject *> m_children;
#else
#define Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(className)
#endif

QT_END_NAMESPACE

#endif // QWAYLANDQUICKCHILDREN_H
