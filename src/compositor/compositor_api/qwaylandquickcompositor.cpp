/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/private/qwlsurface_p.h>

#include "qwaylandclient.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquicksurface.h"
#include "qwaylandsurfaceitem.h"
#include "qwaylandquickoutput.h"

QT_BEGIN_NAMESPACE

class QWaylandQuickCompositorPrivate : public QtWayland::Compositor
{
public:
    QWaylandQuickCompositorPrivate(QWaylandQuickCompositor *compositor, QWaylandCompositor::ExtensionFlags extensions)
        : QtWayland::Compositor(compositor, extensions)
    {
    }

    void compositor_create_surface(Resource *resource, uint32_t id) Q_DECL_OVERRIDE
    {
        QWaylandQuickSurface *surface = new QWaylandQuickSurface(resource->client(), id, wl_resource_get_version(resource->handle), static_cast<QWaylandQuickCompositor *>(m_qt_compositor));
        surface->handle()->addToOutput(primaryOutput()->handle());
        m_surfaces << surface->handle();
        //BUG: This may not be an on-screen window surface though
        m_qt_compositor->surfaceCreated(surface);
    }
};


QWaylandQuickCompositor::QWaylandQuickCompositor(const char *socketName, ExtensionFlags extensions)
                       : QWaylandCompositor(socketName, new QWaylandQuickCompositorPrivate(this, extensions))
{
    qmlRegisterUncreatableType<QWaylandSurfaceItem>("QtCompositor", 1, 0, "WaylandSurfaceItem", QObject::tr("Cannot create instance of WaylandSurfaceItem"));
    qmlRegisterUncreatableType<QWaylandQuickSurface>("QtCompositor", 1, 0, "WaylandQuickSurface", QObject::tr("Cannot create instance of WaylandQuickSurface"));
    qmlRegisterUncreatableType<QWaylandClient>("QtCompositor", 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
    qmlRegisterUncreatableType<QWaylandOutput>("QtCompositor", 1, 0, "WaylandOutput", QObject::tr("Cannot create instance of WaylandOutput"));
}

QWaylandSurfaceView *QWaylandQuickCompositor::createView(QWaylandSurface *surf)
{
    return new QWaylandSurfaceItem(static_cast<QWaylandQuickSurface *>(surf));
}

QWaylandOutput *QWaylandQuickCompositor::createOutput(QWindow *window,
                                                      const QString &manufacturer,
                                                      const QString &model)
{
    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window);
    if (!quickWindow)
        qFatal("%s: couldn't cast QWindow to QQuickWindow. All output windows must "
               "be QQuickWindow derivates when using QWaylandQuickCompositor", Q_FUNC_INFO);
    return new QWaylandQuickOutput(this, quickWindow, manufacturer, model);
}

QT_END_NAMESPACE
