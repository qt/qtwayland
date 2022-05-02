/****************************************************************************
**
** Copyright (C) 2017 Erik Larsson.
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

#ifndef QWAYLANDCLIENTEXTENSION_H
#define QWAYLANDCLIENTEXTENSION_H

#include <QObject>
#include <QtWaylandClient/qtwaylandclientglobal.h>

struct wl_interface;
struct wl_registry;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {
class QWaylandIntegration;
}

class QWaylandClientExtensionPrivate;
class QWaylandClientExtensionTemplatePrivate;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientExtension : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandClientExtension)
    Q_PROPERTY(int protocolVersion READ version NOTIFY versionChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
public:
    QWaylandClientExtension(const int version);
    ~QWaylandClientExtension();

    QtWaylandClient::QWaylandIntegration *integration() const;
    int version() const;
    bool isActive() const;

    virtual const struct wl_interface *extensionInterface() const = 0;
    virtual void bind(struct ::wl_registry *registry, int id, int version) = 0;
protected:
    void setVersion(const int version);
Q_SIGNALS:
    void versionChanged();
    void activeChanged();

protected Q_SLOTS:
    void initialize();
};

template <typename T>
class Q_WAYLAND_CLIENT_EXPORT QWaylandClientExtensionTemplate : public QWaylandClientExtension
{
    Q_DECLARE_PRIVATE(QWaylandClientExtensionTemplate)
public:
    QWaylandClientExtensionTemplate(const int ver) :
        QWaylandClientExtension(ver)
    {
    }

    const struct wl_interface *extensionInterface() const override
    {
        return T::interface();
    }

    void bind(struct ::wl_registry *registry, int id, int ver) override
    {
        T* instance = static_cast<T *>(this);
        // Make sure lowest version is used of the supplied version from the
        // developer and the version specified in the protocol and also the
        // compositor version.
        if (this->version() > T::interface()->version) {
            qWarning("Supplied protocol version to QWaylandClientExtensionTemplate is higher than the version of the protocol, using protocol version instead.");
        }
        int minVersion = qMin(ver, qMin(T::interface()->version, this->version()));
        setVersion(minVersion);
        instance->init(registry, id, minVersion);
    }
};

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTEXTENSION_H
