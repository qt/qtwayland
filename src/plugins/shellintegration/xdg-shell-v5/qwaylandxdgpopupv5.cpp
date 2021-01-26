/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qwaylandxdgpopupv5_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandextendedsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgPopupV5::QWaylandXdgPopupV5(struct ::xdg_popup_v5 *popup, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , QtWayland::xdg_popup_v5(popup)
    , m_window(window)
{
    if (window->display()->windowExtension())
        m_extendedWindow = new QWaylandExtendedSurface(window);
}

QWaylandXdgPopupV5::~QWaylandXdgPopupV5()
{
    xdg_popup_destroy(object());
    delete m_extendedWindow;
}

void QWaylandXdgPopupV5::xdg_popup_popup_done()
{
    m_window->window()->close();
}

}

QT_END_NAMESPACE
