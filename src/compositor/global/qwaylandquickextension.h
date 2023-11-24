// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKEXTENSION_H
#define QWAYLANDQUICKEXTENSION_H

#if 0
#pragma qt_class(QWaylandQuickExtension)
#endif

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlListProperty>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(className) \
    class Q_WAYLANDCOMPOSITOR_EXPORT className##QuickExtension : public className, public QQmlParserStatus \
    { \
/* qmake ignore Q_OBJECT */ \
        Q_OBJECT \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
        Q_INTERFACES(QQmlParserStatus) \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, &m_objects); \
        } \
        void classBegin() override {} \
        void componentComplete() override { if (!isInitialized()) initialize(); } \
    private: \
        QList<QObject *> m_objects; \
    };

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS(className) \
    class Q_WAYLANDCOMPOSITOR_EXPORT className##QuickExtensionContainer : public className \
    { \
/* qmake ignore Q_OBJECT */ \
        Q_OBJECT \
        Q_PROPERTY(QQmlListProperty<QWaylandCompositorExtension> extensions READ extensions) \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, &m_objects); \
        } \
        QQmlListProperty<QWaylandCompositorExtension> extensions() \
        { \
            return QQmlListProperty<QWaylandCompositorExtension>(this, this, \
                                                       &className##QuickExtensionContainer::append_extension, \
                                                       &className##QuickExtensionContainer::countFunction, \
                                                       &className##QuickExtensionContainer::atFunction, \
                                                       &className##QuickExtensionContainer::clearFunction); \
        } \
        static int countFunction(QQmlListProperty<QWaylandCompositorExtension> *list) \
        { \
            return static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.size(); \
        } \
        static QWaylandCompositorExtension *atFunction(QQmlListProperty<QWaylandCompositorExtension> *list, int index) \
        { \
            return static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.at(index); \
        } \
        static void append_extension(QQmlListProperty<QWaylandCompositorExtension> *list, QWaylandCompositorExtension *extension) \
        { \
            className##QuickExtensionContainer *quickExtObj = static_cast<className##QuickExtensionContainer *>(list->data); \
            extension->setExtensionContainer(quickExtObj); \
        } \
        static void clearFunction(QQmlListProperty<QWaylandCompositorExtension> *list) \
        { \
            static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.clear(); \
        } \
    private: \
        QList<QObject *> m_objects; \
    };

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(className, QmlType) \
    Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(className, QmlType, 1, 0)

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(...) \
    QT_OVERLOADED_MACRO(Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT, __VA_ARGS__)

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT_4(className, QmlType, versionMajor, \
                                                             versionMinor)                     \
    class Q_WAYLANDCOMPOSITOR_EXPORT className##QuickExtension : public className,             \
                                                                 public QQmlParserStatus       \
    {                                                                                          \
        /* qmake ignore Q_OBJECT */                                                            \
        Q_OBJECT                                                                               \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)                  \
        Q_CLASSINFO("DefaultProperty", "data")                                                 \
        Q_INTERFACES(QQmlParserStatus)                                                         \
        QML_NAMED_ELEMENT(QmlType)                                                             \
        QML_ADDED_IN_VERSION(versionMajor, versionMinor)                                       \
    public:                                                                                    \
        QQmlListProperty<QObject> data()                                                       \
        {                                                                                      \
            return QQmlListProperty<QObject>(this, &m_objects);                                \
        }                                                                                      \
        void classBegin() override { }                                                         \
        void componentComplete() override                                                      \
        {                                                                                      \
            if (!isInitialized())                                                              \
                initialize();                                                                  \
        }                                                                                      \
                                                                                               \
    private:                                                                                   \
        QList<QObject *> m_objects;                                                            \
    };

#define Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT_2(className, QmlType)         \
    class Q_WAYLANDCOMPOSITOR_EXPORT className##QuickExtension : public className,       \
                                                                 public QQmlParserStatus \
    {                                                                                    \
        /* qmake ignore Q_OBJECT */                                                      \
        Q_OBJECT                                                                         \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)            \
        Q_CLASSINFO("DefaultProperty", "data")                                           \
        Q_INTERFACES(QQmlParserStatus)                                                   \
        QML_NAMED_ELEMENT(QmlType)                                                       \
    public:                                                                              \
        QQmlListProperty<QObject> data()                                                 \
        {                                                                                \
            return QQmlListProperty<QObject>(this, &m_objects);                          \
        }                                                                                \
        void classBegin() override { }                                                   \
        void componentComplete() override                                                \
        {                                                                                \
            if (!isInitialized())                                                        \
                initialize();                                                            \
        }                                                                                \
                                                                                         \
    private:                                                                             \
        QList<QObject *> m_objects;                                                      \
    };

QT_END_NAMESPACE

#endif  /*QWAYLANDQUICKEXTENSION_H*/
