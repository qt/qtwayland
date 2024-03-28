// Copyright (C) 2022 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKCOMPOSITOR_VIEWPORT_H
#define MOCKCOMPOSITOR_VIEWPORT_H

#include "coreprotocol.h"
#include <qwayland-server-viewporter.h>

namespace MockCompositor {

class Viewport;

class Viewporter : public Global, public QtWaylandServer::wp_viewporter
{
    Q_OBJECT
public:
    explicit Viewporter(CoreCompositor *compositor, int version = 1);
    QList<Viewport *> m_viewports;

protected:
    void wp_viewporter_get_viewport(Resource *resource, uint32_t id, wl_resource *surface) override;
};

class Viewport : public QObject, public QtWaylandServer::wp_viewport
{
    Q_OBJECT
public:
    explicit Viewport(Surface *surface, wl_client *client, int id, int version);

    QRectF m_source;
    QSize m_destination;

    Surface* m_surface;

Q_SIGNALS:
    void sourceChanged();
    void destinationChanged();

protected:
    void wp_viewport_set_source(Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height) override;
    void wp_viewport_set_destination(Resource *resource, int32_t width, int32_t height) override;

    void wp_viewport_destroy_resource(Resource *resource) override;
    void wp_viewport_destroy(Resource *resource) override;
};

}

#endif
