// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDIDLEINHIBITV1_P_H
#define QWAYLANDIDLEINHIBITV1_P_H

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandIdleInhibitManagerV1>
#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-idle-inhibit-unstable-v1.h>

#include <QtCore/qpointer.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandIdleInhibitManagerV1Private
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::zwp_idle_inhibit_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandIdleInhibitManagerV1)
public:
    explicit QWaylandIdleInhibitManagerV1Private() = default;

    class Q_WAYLANDCOMPOSITOR_EXPORT Inhibitor
            : public QtWaylandServer::zwp_idle_inhibitor_v1
    {
    public:
        explicit Inhibitor(QWaylandSurface *surface, wl_client *client, quint32 id, quint32 version);

    protected:
        void zwp_idle_inhibitor_v1_destroy_resource(Resource *resource) override;
        void zwp_idle_inhibitor_v1_destroy(Resource *resource) override;

    private:
        QPointer<QWaylandSurface> m_surface;
    };

    static QWaylandIdleInhibitManagerV1Private *get(QWaylandIdleInhibitManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zwp_idle_inhibit_manager_v1_create_inhibitor(Resource *resource, uint32_t id, wl_resource *surfaceResource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDIDLEINHIBITV1_P_H
