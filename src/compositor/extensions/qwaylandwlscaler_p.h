/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDWLSCALER_P_H
#define QWAYLANDWLSCALER_P_H

#include "qwaylandwlscaler.h"

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-scaler.h>

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

#if QT_DEPRECATED_SINCE(5, 13)
class QWaylandSurface;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandWlScalerPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::wl_scaler
{
    Q_DECLARE_PUBLIC(QWaylandWlScaler)
public:
    explicit QWaylandWlScalerPrivate() = default;

protected:
    void scaler_destroy(Resource *resource) override;
    void scaler_get_viewport(Resource *resource, uint32_t id, wl_resource *surface) override;

private:
    class Viewport : public QtWaylandServer::wl_viewport
    {
    public:
        explicit Viewport(QWaylandSurface *surface, wl_client *client, int id, int version);
        void checkCommittedState();

    protected:
        void viewport_destroy_resource(Resource *resource) override;
        void viewport_destroy(Resource *resource) override;
        void viewport_set(Resource *resource, wl_fixed_t src_x, wl_fixed_t src_y, wl_fixed_t src_width, wl_fixed_t src_height, int32_t dst_width, int32_t dst_height) override;
        void viewport_set_source(Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height) override;
        void viewport_set_destination(Resource *resource, int32_t width, int32_t height) override;

    private:
        QPointer<QWaylandSurface> m_surface = nullptr;
    };
};
#endif

QT_END_NAMESPACE

#endif // QWAYLANDWLSCALER_P_H
