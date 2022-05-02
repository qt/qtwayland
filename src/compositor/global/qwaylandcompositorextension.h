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

#ifndef QWAYLANDEXTENSION_H
#define QWAYLANDEXTENSION_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

#include <QtCore/QObject>

struct wl_interface;

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandCompositorExtension;
class QWaylandCompositorExtensionPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandObject : public QObject
{
    Q_OBJECT
public:
    ~QWaylandObject() override;

    QWaylandCompositorExtension *extension(const QByteArray &name);
    QWaylandCompositorExtension *extension(const wl_interface *interface);
    QList<QWaylandCompositorExtension *> extensions() const;
    void addExtension(QWaylandCompositorExtension *extension);
    void removeExtension(QWaylandCompositorExtension *extension);

protected:
    QWaylandObject(QObject *parent = nullptr);
    QWaylandObject(QObjectPrivate &d, QObject *parent = nullptr);
    QList<QWaylandCompositorExtension *> extension_vector;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandCompositorExtension : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandCompositorExtension)
    QML_NAMED_ELEMENT(WaylandExtension)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    QWaylandCompositorExtension();
    QWaylandCompositorExtension(QWaylandObject *container);
    ~QWaylandCompositorExtension() override;

    QWaylandObject *extensionContainer() const;
    void setExtensionContainer(QWaylandObject *container);

    virtual void initialize();
    bool isInitialized() const;

    virtual const struct wl_interface *extensionInterface() const = 0;

protected:
    QWaylandCompositorExtension(QWaylandCompositorExtensionPrivate &dd);
    QWaylandCompositorExtension(QWaylandObject *container, QWaylandCompositorExtensionPrivate &dd);

    bool event(QEvent *event) override;
};

template <typename T>
class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandCompositorExtensionTemplate : public QWaylandCompositorExtension
{
public:
    QWaylandCompositorExtensionTemplate()
        : QWaylandCompositorExtension()
    { }

    QWaylandCompositorExtensionTemplate(QWaylandObject *container)
        : QWaylandCompositorExtension(container)
    { }

    const struct wl_interface *extensionInterface() const override
    {
        return T::interface();
    }

    static T *findIn(QWaylandObject *container)
    {
        if (!container) return nullptr;
        return qobject_cast<T *>(container->extension(T::interfaceName()));
    }

protected:
    QWaylandCompositorExtensionTemplate(QWaylandCompositorExtensionPrivate &dd)
        : QWaylandCompositorExtension(dd)
    { }

    QWaylandCompositorExtensionTemplate(QWaylandObject *container, QWaylandCompositorExtensionPrivate &dd)
        : QWaylandCompositorExtension(container,dd)
    { }
};

QT_END_NAMESPACE

#endif
