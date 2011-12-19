/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDEXTENDEDSURFACE_H
#define QWAYLANDEXTENDEDSURFACE_H

#include <wayland-client.h>

#include <QtCore/QString>
#include <QtCore/QVariant>

class QWaylandDisplay;
class QWaylandWindow;
class QWaylandExtendedSurface;

class QWaylandSurfaceExtension
{
public:
    QWaylandSurfaceExtension(QWaylandDisplay *display, uint32_t id);

    QWaylandExtendedSurface *getExtendedWindow(QWaylandWindow *window);
private:
    struct wl_surface_extension *m_surface_extension;
};

class QWaylandExtendedSurface
{
public:
    QWaylandExtendedSurface(QWaylandWindow *window, struct wl_extended_surface *extended_surface);

    void updateGenericProperty(const QString &name, const QVariant &value);
    QVariantMap properties() const;
    QVariant property(const QString &name);
    QVariant property(const QString &name, const QVariant &defaultValue);
private:
    QWaylandWindow *m_window;
    struct wl_extended_surface *m_extended_surface;

    QVariantMap m_properties;

    static void onscreen_visibility(void *data,
                                struct wl_extended_surface *wl_extended_surface,
                                int32_t visible);

    static void set_generic_property(void *data,
                                 struct wl_extended_surface *wl_extended_surface,
                                 const char *name,
                                 struct wl_array *value);

    static const struct wl_extended_surface_listener extended_surface_listener;

};

#endif // QWAYLANDEXTENDEDSURFACE_H
