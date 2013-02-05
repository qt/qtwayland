/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandextendedsurface.h"

#include "qwaylandwindow.h"

#include "wayland-client.h"
#include "wayland-surface-extension-client-protocol.h"

#include "qwaylanddisplay.h"

#include "qwaylandnativeinterface.h"

#include <QtGui/QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qwindowsysteminterface.h>

QT_USE_NAMESPACE

QWaylandSurfaceExtension::QWaylandSurfaceExtension(QWaylandDisplay *display, uint32_t id)
{
    m_surface_extension = static_cast<struct wl_surface_extension *>(
                wl_registry_bind(display->wl_registry(), id, &wl_surface_extension_interface, 1));
}

QWaylandExtendedSurface *QWaylandSurfaceExtension::getExtendedWindow(QWaylandWindow *window)
{
    struct wl_surface *surface = window->wl_surface();
    Q_ASSERT(surface);
    struct wl_extended_surface *extended_surface =
            wl_surface_extension_get_extended_surface(m_surface_extension,surface);

    return new QWaylandExtendedSurface(window,extended_surface);
}


QWaylandExtendedSurface::QWaylandExtendedSurface(QWaylandWindow *window, struct wl_extended_surface *extended_surface)
    : m_window(window)
    , m_extended_surface(extended_surface)
    , m_exposed(true)
{
    wl_extended_surface_add_listener(m_extended_surface,&QWaylandExtendedSurface::extended_surface_listener,this);
}

void QWaylandExtendedSurface::updateGenericProperty(const QString &name, const QVariant &value)
{

    QByteArray byteValue;
    QDataStream ds(&byteValue, QIODevice::WriteOnly);
    ds << value;

    struct wl_array data;
    data.size = byteValue.size();
    data.data = (void*)byteValue.constData();
    data.alloc = 0;

    wl_extended_surface_update_generic_property(m_extended_surface,qPrintable(name),&data);

    m_properties.insert(name,value);
    QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(
                QGuiApplication::platformNativeInterface());
    nativeInterface->emitWindowPropertyChanged(m_window,name);
}

static int32_t waylandRotationFromScreenOrientation(Qt::ScreenOrientation orientation)
{
    switch (orientation) {
    case Qt::PortraitOrientation: return WL_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION;
    case Qt::InvertedPortraitOrientation: return WL_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION;
    case Qt::LandscapeOrientation: return WL_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION;
    case Qt::InvertedLandscapeOrientation: return WL_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION;
    default: return WL_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION;
    }
}

void QWaylandExtendedSurface::setContentOrientation(Qt::ScreenOrientation orientation)
{
    wl_extended_surface_set_content_orientation(m_extended_surface, waylandRotationFromScreenOrientation(orientation));
}

QVariantMap QWaylandExtendedSurface::properties() const
{
    return m_properties;
}

QVariant QWaylandExtendedSurface::property(const QString &name)
{
    return m_properties.value(name);
}

QVariant QWaylandExtendedSurface::property(const QString &name, const QVariant &defaultValue)
{
    return m_properties.value(name,defaultValue);
}

void QWaylandExtendedSurface::onscreen_visibility(void *data, wl_extended_surface *wl_extended_surface, int32_t visible)
{
    QWaylandExtendedSurface *extendedWindow = static_cast<QWaylandExtendedSurface *>(data);
    Q_UNUSED(extendedWindow);
    Q_UNUSED(wl_extended_surface);

    // Do not send events when the state is not changing...
    if (visible == extendedWindow->m_window->isExposed())
        return;

    extendedWindow->m_exposed = visible;
    QWaylandWindow *w = extendedWindow->m_window;
    QWindowSystemInterface::handleExposeEvent(w->window(),
                                              visible
                                              ? QRegion(w->geometry())
                                              : QRegion());
    QWindowSystemInterface::flushWindowSystemEvents();
}

void QWaylandExtendedSurface::set_generic_property(void *data, wl_extended_surface *wl_extended_surface, const char *name, wl_array *value)
{
    Q_UNUSED(wl_extended_surface);

    QWaylandExtendedSurface *extended_window = static_cast<QWaylandExtendedSurface *>(data);

    QVariant variantValue;
    QByteArray baValue = QByteArray((const char*)value->data, value->size);
    QDataStream ds(&baValue, QIODevice::ReadOnly);
    ds >> variantValue;

    QString qstring_name = QString::fromLatin1(name);
    extended_window->m_properties.insert(qstring_name,variantValue);

    QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(
                QGuiApplication::platformNativeInterface());
    nativeInterface->emitWindowPropertyChanged(extended_window->m_window,QString::fromLatin1(name));
}

Qt::WindowFlags QWaylandExtendedSurface::setWindowFlags(Qt::WindowFlags flags)
{
    uint wlFlags = 0;
    if (flags & Qt::WindowStaysOnTopHint) wlFlags |= WL_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP;
    if (flags & Qt::WindowOverridesSystemGestures) wlFlags |= WL_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES;

    wl_extended_surface_set_window_flags(m_extended_surface, wlFlags);

    return flags & (
                    Qt::WindowStaysOnTopHint
                    | Qt::WindowOverridesSystemGestures
                   );
}

const struct wl_extended_surface_listener QWaylandExtendedSurface::extended_surface_listener = {
    QWaylandExtendedSurface::onscreen_visibility,
    QWaylandExtendedSurface::set_generic_property
};
