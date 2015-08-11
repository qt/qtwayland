/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef QTWAYLAND_QWLINPUTPANELSURFACE_P_H
#define QTWAYLAND_QWLINPUTPANELSURFACE_P_H

#include <QtCompositor/private/qwayland-server-input-method.h>

#include <QWaylandSurface>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Output;

class InputPanelSurface : public QtWaylandServer::wl_input_panel_surface
{
public:
    enum Type {
        Invalid,
        Toplevel,
        OverlayPanel
    };

    InputPanelSurface(struct ::wl_client *client, int id, QWaylandSurface *surface);

    Type type() const;

    Output *output() const;
    wl_input_panel_surface::position position() const;

protected:
    void input_panel_surface_set_overlay_panel(Resource *resource) Q_DECL_OVERRIDE;
    void input_panel_surface_set_toplevel(Resource *resource, wl_resource *output_resource, uint32_t position) Q_DECL_OVERRIDE;

private:
    QWaylandSurface *m_surface;

    Type m_type;

    Output *m_output;
    wl_input_panel_surface::position m_position;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLINPUTPANELSURFACE_P_H
