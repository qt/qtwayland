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

#include "qwlinputpanelsurface_p.h"

#include "qwloutput_p.h"

#include <QtCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

InputPanelSurface::InputPanelSurface(wl_client *client, int id, QWaylandSurface *surface)
    : QtWaylandServer::wl_input_panel_surface(client, id, 1)
    , m_surface(surface)
    , m_type(Invalid)
    , m_output(0)
    , m_position()
{
    QWaylandSurfacePrivate::get(surface)->setInputPanelSurface(this);
}

InputPanelSurface::Type InputPanelSurface::type() const
{
    return m_type;
}

Output *InputPanelSurface::output() const
{
    return m_output;
}

QtWaylandServer::wl_input_panel_surface::position InputPanelSurface::position() const
{
    return m_position;
}

void InputPanelSurface::input_panel_surface_set_overlay_panel(Resource *)
{
    m_type = OverlayPanel;
}

void InputPanelSurface::input_panel_surface_set_toplevel(Resource *, wl_resource *output_resource, uint32_t position)
{
    m_type = Toplevel;
    OutputResource *output = static_cast<OutputResource *>(Output::Resource::fromResource(output_resource));
    m_output = static_cast<Output *>(output->output_object);
    m_position = static_cast<wl_input_panel_surface::position>(position);
}

QT_END_NAMESPACE

} // namespace QtWayland
