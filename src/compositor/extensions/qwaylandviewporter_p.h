/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandViewporterPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::wp_viewporter
{
    Q_DECLARE_PUBLIC(QWaylandViewporter)
public:
    explicit QWaylandViewporterPrivate() = default;

    class Q_WAYLAND_COMPOSITOR_EXPORT Viewport
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
