// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKCOMPOSITOR_IVIAPPLICATION_H
#define MOCKCOMPOSITOR_IVIAPPLICATION_H

#include "coreprotocol.h"
#include <qwayland-server-ivi-application.h>

#include <QList>

namespace MockCompositor {

class Surface;
class IviApplication;
class IviSurface;

class IviApplication : public Global, public QtWaylandServer::ivi_application
{
    Q_OBJECT
public:
    explicit IviApplication(CoreCompositor *compositor);

    QList<IviSurface *> m_iviSurfaces;
protected:
    void ivi_application_surface_create(Resource *resource, uint32_t ivi_id, struct ::wl_resource *surface, uint32_t id) override;

};

class IviSurface : public QObject, public QtWaylandServer::ivi_surface
{
    Q_OBJECT
public:
    IviSurface(IviApplication *iviApplication, Surface *surface, uint32_t ivi_id, wl_client *client, int id, int version);

    Surface *surface() const { return m_surface; }

    void ivi_surface_destroy_resource(Resource *resource) override;
    void ivi_surface_destroy(Resource *resource) override;

    const uint m_iviId = 0;
private:
    IviApplication *m_iviApplication;
    Surface *m_surface = nullptr;
};


} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_IVIAPPLICATION_H
