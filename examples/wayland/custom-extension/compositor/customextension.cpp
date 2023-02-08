// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "customextension.h"

#include <QWaylandSurface>

#include <QDebug>

CustomExtension::CustomExtension(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor)
{
}

void CustomExtension::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);
}

//! [setFontSize]
void CustomExtension::setFontSize(QWaylandSurface *surface, uint pixelSize)
{
    if (surface) {
        Resource *target = resourceMap().value(surface->waylandClient());
        if (target) {
            qDebug() << "Server-side extension sending setFontSize:" << pixelSize;
            send_set_font_size(target->handle,  surface->resource(), pixelSize);
        }
    }
}
//! [setFontSize]

void CustomExtension::showDecorations(QWaylandClient *client, bool shown)
{
    if (client) {
        Resource *target = resourceMap().value(client->client());
        if (target) {
            qDebug() << "Server-side extension sending showDecorations:" << shown;
            send_set_window_decoration(target->handle, shown);
        }
    }
}

void CustomExtension::close(QWaylandSurface *surface)
{
    if (surface) {
        Resource *target = resourceMap().value(surface->waylandClient());
        if (target) {
            qDebug() << "Server-side extension sending close for" << surface;
            send_close(target->handle,  surface->resource());
        }
    }
}

//! [example_extension_bounce]
void CustomExtension::example_extension_bounce(QtWaylandServer::qt_example_extension::Resource *resource, wl_resource *wl_surface, uint32_t duration)
{
    Q_UNUSED(resource);
    auto surface = QWaylandSurface::fromResource(wl_surface);
    qDebug() << "server received bounce" << surface << duration;
    emit bounce(surface, duration);
}
//! [example_extension_bounce]

void CustomExtension::example_extension_spin(QtWaylandServer::qt_example_extension::Resource *resource, wl_resource *wl_surface, uint32_t duration)
{
    Q_UNUSED(resource);
    auto surface = QWaylandSurface::fromResource(wl_surface);
    qDebug() << "server received spin" << surface << duration;
    emit spin(surface, duration);
}

void CustomExtension::example_extension_register_surface(QtWaylandServer::qt_example_extension::Resource *resource, wl_resource *wl_surface)
{
    Q_UNUSED(resource);
    auto surface = QWaylandSurface::fromResource(wl_surface);
    qDebug() << "server received new surface" << surface;
    emit surfaceAdded(surface);
}


void CustomExtension::example_extension_create_local_object(Resource *resource, uint32_t id, const QString &color, const QString &text)
{
    auto *obj = new CustomExtensionObject(color, text, resource->client(), id, 1);
    qDebug() << "Object created" << text << color;
    emit customObjectCreated(obj);
}

CustomExtensionObject::CustomExtensionObject(const QString &color, const QString &text, wl_client *client, int id, int version)
    : QtWaylandServer::qt_example_local_object(client, id, version)
    , m_color(color)
    , m_text(text)
{
}

void CustomExtensionObject::sendClicked()
{
    send_clicked();
}

void CustomExtensionObject::example_local_object_destroy_resource(QtWaylandServer::qt_example_local_object::Resource *resource)
{
    Q_UNUSED(resource);
    qDebug() << "Object destroyed" << m_text << m_color;
    emit resourceDestroyed();
}


void CustomExtensionObject::example_local_object_set_text(QtWaylandServer::qt_example_local_object::Resource *resource, const QString &text)
{
    Q_UNUSED(resource);
    qDebug() << "Client changed text from" << m_text << "to" << text;
    setText(text);
}
