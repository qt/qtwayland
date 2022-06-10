// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WL_REGION_H
#define WL_REGION_H

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

#include <QRegion>
#include <private/qglobal_p.h>

#include <wayland-util.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Q_WAYLANDCOMPOSITOR_EXPORT Region : public QtWaylandServer::wl_region
{
public:
    Region(struct wl_client *client, uint32_t id);
    ~Region() override;

    static Region *fromResource(struct ::wl_resource *resource);

    uint id() const { return wl_resource_get_id(resource()->handle); }

    QRegion region() const { return m_region; }

private:
    Q_DISABLE_COPY(Region)

    QRegion m_region;

    void region_destroy_resource(Resource *) override;

    void region_destroy(Resource *resource) override;
    void region_add(Resource *resource, int32_t x, int32_t y, int32_t w, int32_t h) override;
    void region_subtract(Resource *resource, int32_t x, int32_t y, int32_t w, int32_t h) override;
};

}

QT_END_NAMESPACE

#endif // WL_REGION_H

