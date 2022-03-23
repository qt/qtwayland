// Copyright (C) 2022 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MOCKCOMPOSITOR_FRACTIONALSCALE_H
#define MOCKCOMPOSITOR_FRACTIONALSCALE_H

#include "coreprotocol.h"
#include <qwayland-server-fractional-scale-v1.h>

namespace MockCompositor {

class FractionalScale;

class FractionalScaleManager : public Global, public QtWaylandServer::wp_fractional_scale_manager_v1
{
    Q_OBJECT
public:
    explicit FractionalScaleManager(CoreCompositor *compositor, int version = 1);
    QList<FractionalScale *> m_fractionalScales;

protected:
    void wp_fractional_scale_manager_v1_get_fractional_scale(Resource *resource, uint32_t id, wl_resource *surface) override;
};

class FractionalScale : public QObject, public QtWaylandServer::wp_fractional_scale_v1
{
    Q_OBJECT
public:
    explicit FractionalScale(Surface *surface, wl_client *client, int id, int version);
    Surface *m_surface;

protected:
    void wp_fractional_scale_v1_destroy_resource(Resource *resource) override;
    void wp_fractional_scale_v1_destroy(Resource *resource) override;
};

}

#endif
