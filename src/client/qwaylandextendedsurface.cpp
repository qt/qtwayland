// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
    : QtWayland::qt_extended_surface(window->display()->windowExtension()->get_extended_surface(window->wlSurface()))
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
