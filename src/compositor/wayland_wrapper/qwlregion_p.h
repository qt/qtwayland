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

#include <wayland-util.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Q_WAYLAND_COMPOSITOR_EXPORT Region : public QtWaylandServer::wl_region
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

