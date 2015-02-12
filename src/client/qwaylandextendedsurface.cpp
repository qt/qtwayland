/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandextendedsurface_p.h"

#include "qwaylandwindow_p.h"

#include "qwaylanddisplay_p.h"

#include "qwaylandnativeinterface_p.h"

#include <QtGui/QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandExtendedSurface::QWaylandExtendedSurface(QWaylandWindow *window)
    : QtWayland::qt_extended_surface(window->display()->windowExtension()->get_extended_surface(window->object()))
    , m_window(window)
{
}

QWaylandExtendedSurface::~QWaylandExtendedSurface()
{
    qt_extended_surface_destroy(object());
}

void QWaylandExtendedSurface::updateGenericProperty(const QString &name, const QVariant &value)
{
    QByteArray byteValue;
    QDataStream ds(&byteValue, QIODevice::WriteOnly);
    ds << value;

    update_generic_property(name, byteValue);
}

void QWaylandExtendedSurface::setContentOrientationMask(Qt::ScreenOrientations mask)
{
    int32_t wlmask = 0;
    if (mask & Qt::PrimaryOrientation)
        wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION;
    if (mask & Qt::PortraitOrientation)
        wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION;
    if (mask & Qt::LandscapeOrientation)
        wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION;
    if (mask & Qt::InvertedPortraitOrientation)
        wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION;
    if (mask & Qt::InvertedLandscapeOrientation)
        wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION;
    set_content_orientation_mask(wlmask);
}

void QWaylandExtendedSurface::extended_surface_onscreen_visibility(int32_t visibility)
{
    m_window->window()->setVisibility(static_cast<QWindow::Visibility>(visibility));
}

void QWaylandExtendedSurface::extended_surface_set_generic_property(const QString &name, wl_array *value)
{
    QByteArray data = QByteArray::fromRawData(static_cast<char *>(value->data), value->size);

    QVariant variantValue;
    QDataStream ds(data);
    ds >> variantValue;

    m_window->setProperty(name, variantValue);
}

void QWaylandExtendedSurface::extended_surface_close()
{
    QWindowSystemInterface::handleCloseEvent(m_window->window());
}

Qt::WindowFlags QWaylandExtendedSurface::setWindowFlags(Qt::WindowFlags flags)
{
    uint wlFlags = 0;

    if (flags & Qt::WindowStaysOnTopHint) wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP;
    if (flags & Qt::WindowOverridesSystemGestures) wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES;
    if (flags & Qt::BypassWindowManagerHint) wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_BYPASSWINDOWMANAGER;

    set_window_flags(wlFlags);

    return flags & (Qt::WindowStaysOnTopHint | Qt::WindowOverridesSystemGestures | Qt::BypassWindowManagerHint);
}

}

QT_END_NAMESPACE
