/****************************************************************************
**
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

#include "qwlinputpanel_p.h"

#include <QtCompositor/qwaylandinputpanel.h>

#include "qwlcompositor_p.h"
#include "qwlinputdevice_p.h"
#include "qwlinputmethod_p.h"
#include "qwlinputpanelsurface_p.h"
#include "qwlsurface_p.h"
#include "qwltextinput_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

InputPanel::InputPanel(Compositor *compositor)
    : QtWaylandServer::wl_input_panel(compositor->wl_display(), 1)
    , m_compositor(compositor)
    , m_handle(new QWaylandInputPanel(this))
    , m_focus()
    , m_inputPanelVisible(false)
    , m_cursorRectangle()
{
}

InputPanel::~InputPanel()
{
}

QWaylandInputPanel *InputPanel::handle() const
{
    return m_handle.data();
}

Surface *InputPanel::focus() const
{
    return m_focus;
}

void InputPanel::setFocus(Surface *focus)
{
    if (m_focus == focus)
        return;

    m_focus = focus;

    Q_EMIT handle()->focusChanged();
}

bool InputPanel::inputPanelVisible() const
{
    return m_inputPanelVisible;
}

void InputPanel::setInputPanelVisible(bool inputPanelVisible)
{
    if (m_inputPanelVisible == inputPanelVisible)
        return;

    m_inputPanelVisible = inputPanelVisible;

    Q_EMIT handle()->visibleChanged();
}

QRect InputPanel::cursorRectangle() const
{
    return m_cursorRectangle;
}

void InputPanel::setCursorRectangle(const QRect &cursorRectangle)
{
    if (m_cursorRectangle == cursorRectangle)
        return;

    m_cursorRectangle = cursorRectangle;

    Q_EMIT handle()->cursorRectangleChanged();
}

void InputPanel::input_panel_get_input_panel_surface(Resource *resource, uint32_t id, wl_resource *surface)
{
    new InputPanelSurface(resource->client(), id, Surface::fromResource(surface));
}

} // namespace QtWayland

QT_END_NAMESPACE
