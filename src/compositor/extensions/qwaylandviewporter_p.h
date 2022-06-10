// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDVIEWPORTER_P_H
#define QWAYLANDVIEWPORTER_P_H

#include "qwaylandviewporter.h"

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-viewporter.h>

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

QT_BEGIN_NAMESPACE

class QWaylandSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandViewporterPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::wp_viewporter
{
    Q_DECLARE_PUBLIC(QWaylandViewporter)
public:
    explicit QWaylandViewporterPrivate() = default;

    class Q_WAYLANDCOMPOSITOR_EXPORT Viewport
            : public QtWaylandServer::wp_viewport
    {
    public:
        explicit Viewport(QWaylandSurface *surface, wl_client *client, int id);
        ~Viewport() override;
        void checkCommittedState();

    protected:
        void wp_viewport_destroy_resource(Resource *resource) override;
        void wp_viewport_destroy(Resource *resource) override;
        void wp_viewport_set_source(Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height) override;
        void wp_viewport_set_destination(Resource *resource, int32_t width, int32_t height) override;

    private:
        QPointer<QWaylandSurface> m_surface = nullptr;
    };

protected:
    void wp_viewporter_destroy(Resource *resource) override;
    void wp_viewporter_get_viewport(Resource *resource, uint32_t id, wl_resource *surface) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDVIEWPORTER_P_H
