// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CUSTOMEXTENSION_H
#define CUSTOMEXTENSION_H

#include "wayland-util.h"

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include "qwayland-server-custom.h"

class CustomExtensionObject;

//! [CustomExtension]
class CustomExtension  : public QWaylandCompositorExtensionTemplate<CustomExtension>
        , public QtWaylandServer::qt_example_extension
{
    Q_OBJECT
    QML_ELEMENT
//! [CustomExtension]
public:
    CustomExtension(QWaylandCompositor *compositor = nullptr);
    void initialize() override;

signals:
    void surfaceAdded(QWaylandSurface *surface);
    void bounce(QWaylandSurface *surface, uint ms);
    void spin(QWaylandSurface *surface, uint ms);
    void customObjectCreated(CustomExtensionObject *obj);

public slots:
    void setFontSize(QWaylandSurface *surface, uint pixelSize);
    void showDecorations(QWaylandClient *client, bool);
    void close(QWaylandSurface *surface);

//! [example_extension_bounce]
protected:
    void example_extension_bounce(Resource *resource, wl_resource *surface, uint32_t duration) override;
//! [example_extension_bounce]
    void example_extension_spin(Resource *resource, wl_resource *surface, uint32_t duration) override;
    void example_extension_register_surface(Resource *resource, wl_resource *surface) override;
    void example_extension_create_local_object(Resource *resource, uint32_t id, const QString &color, const QString &text) override;
};


class CustomExtensionObject : public QWaylandCompositorExtensionTemplate<CustomExtensionObject>
        , public QtWaylandServer::qt_example_local_object
{
    Q_OBJECT
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    CustomExtensionObject(const QString &color, const QString &text, struct ::wl_client *client, int id, int version);

    QString color() const
    {
        return m_color;
    }

    QString text() const
    {
        return m_text;
    }

public slots:
    void setColor(const QString &color)
    {
        if (m_color == color)
            return;

        m_color = color;
        emit colorChanged(m_color);
    }

    void setText(QString text)
    {
        if (m_text == text)
            return;

        m_text = text;
        emit textChanged(m_text);
    }
    void sendClicked();

signals:
    void colorChanged(const QString &color);
    void resourceDestroyed();

    void textChanged(QString text);

protected:
    void example_local_object_destroy_resource(Resource *resource) override;
    void example_local_object_set_text(Resource *resource, const QString &text) override;

private:
    QString m_color;
    QString m_text;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(CustomExtension)

#endif // CUSTOMEXTENSION_H
