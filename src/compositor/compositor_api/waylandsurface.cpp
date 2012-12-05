/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandsurface.h"

#include <private/qobject_p.h>

#include "wayland_wrapper/wlsurface.h"
#include "wayland_wrapper/wlextendedsurface.h"
#include "wayland_wrapper/wlsubsurface.h"
#include "wayland_wrapper/wlcompositor.h"
#include "wayland_wrapper/wlshellsurface.h"

#include "waylandcompositor.h"
#include "waylandwindowmanagerintegration.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#ifdef QT_COMPOSITOR_QUICK
#include "waylandsurfaceitem.h"
#endif

class WaylandSurfacePrivate : public QObjectPrivate
{
public:
    WaylandSurfacePrivate(Wayland::Surface *srfc)
        : surface(srfc)
#ifdef QT_COMPOSITOR_QUICK
        , surface_item(0)
#endif
    {}

    ~WaylandSurfacePrivate()
    {
#ifdef QT_COMPOSITOR_QUICK
        if (surface_item)
            surface_item->setSurface(0);
#endif
    }

    Wayland::Surface *surface;
#ifdef QT_COMPOSITOR_QUICK
    WaylandSurfaceItem *surface_item;
#endif
};

WaylandSurface::WaylandSurface(Wayland::Surface *surface)
    : QObject(*new WaylandSurfacePrivate(surface))
{
}

WaylandClient *WaylandSurface::client() const
{
    Q_D(const WaylandSurface);
    return d->surface->base()->resource.client;
}

WaylandSurface *WaylandSurface::parentSurface() const
{
    Q_D(const WaylandSurface);
    if (d->surface->subSurface() && d->surface->subSurface()->parent()) {
        return d->surface->subSurface()->parent()->waylandSurface();
    }
    return 0;
}

QLinkedList<WaylandSurface *> WaylandSurface::subSurfaces() const
{
    Q_D(const WaylandSurface);
    if (d->surface->subSurface()) {
        return d->surface->subSurface()->subSurfaces();
    }
    return QLinkedList<WaylandSurface *>();
}

WaylandSurface::Type WaylandSurface::type() const
{
    Q_D(const WaylandSurface);
    return d->surface->type();
}

bool WaylandSurface::isYInverted() const
{
    Q_D(const WaylandSurface);
    return d->surface->isYInverted();
}

bool WaylandSurface::visible() const
{
    Q_D(const WaylandSurface);
    return d->surface->visible();
}

QPointF WaylandSurface::pos() const
{
    Q_D(const WaylandSurface);
    return d->surface->pos();
}

void WaylandSurface::setPos(const QPointF &pos)
{
    Q_D(WaylandSurface);
    d->surface->setPos(pos);
}

QSize WaylandSurface::size() const
{
    Q_D(const WaylandSurface);
    return d->surface->size();
}

void WaylandSurface::setSize(const QSize &size)
{
    Q_D(WaylandSurface);
    d->surface->setSize(size);
}

void WaylandSurface::sendConfigure(const QSize &size)
{
    Q_D(WaylandSurface);
    if (d->surface->shellSurface())
        d->surface->shellSurface()->sendConfigure(WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT, size.width(), size.height());
}

Qt::ScreenOrientations WaylandSurface::orientationUpdateMask() const
{
    Q_D(const WaylandSurface);
    return d->surface->compositor()->orientationUpdateMaskForClient(static_cast<wl_client *>(client()));
}

Qt::ScreenOrientation WaylandSurface::contentOrientation() const
{
    Q_D(const WaylandSurface);
    if (!d->surface->extendedSurface())
        return Qt::PrimaryOrientation;
    return d->surface->extendedSurface()->contentOrientation();
}

WaylandSurface::WindowFlags WaylandSurface::windowFlags() const
{
    Q_D(const WaylandSurface);
    if (!d->surface->extendedSurface())
        return WaylandSurface::WindowFlags(0);
    return d->surface->extendedSurface()->windowFlags();
}


QImage WaylandSurface::image() const
{
    Q_D(const WaylandSurface);
    return d->surface->image();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint WaylandSurface::texture(QOpenGLContext *context) const
{
    Q_D(const WaylandSurface);
    return d->surface->textureId(context);
}
#else //QT_COMPOSITOR_WAYLAND_GL
uint WaylandSurface::texture(QOpenGLContext *) const
{
    return 0;
}
#endif

Wayland::Surface * WaylandSurface::handle() const
{
    Q_D(const WaylandSurface);
    return d->surface;
}

#ifdef QT_COMPOSITOR_QUICK
WaylandSurfaceItem *WaylandSurface::surfaceItem() const
{
    Q_D(const WaylandSurface);
    return d->surface_item;
}

void WaylandSurface::setSurfaceItem(WaylandSurfaceItem *surfaceItem)
{
    Q_D(WaylandSurface);
    d->surface_item = surfaceItem;
}
#endif //QT_COMPOSITOR_QUICK

qint64 WaylandSurface::processId() const
{
    Q_D(const WaylandSurface);
    WindowManagerServerIntegration *wmIntegration = d->surface->compositor()->windowManagerIntegration();
    if (!wmIntegration) {
        return 0;
    }

    WaylandManagedClient *mcl = wmIntegration->managedClient(d->surface->base()->resource.client);
    return mcl ? mcl->processId() : 0;
}

QByteArray WaylandSurface::authenticationToken() const
{
    Q_D(const WaylandSurface);
    WindowManagerServerIntegration *wmIntegration = d->surface->compositor()->windowManagerIntegration();
    if (!wmIntegration) {
        return QByteArray();
    }

    WaylandManagedClient *mcl = wmIntegration->managedClient(d->surface->base()->resource.client);
    return mcl ? mcl->authenticationToken() : QByteArray();
}

QVariantMap WaylandSurface::windowProperties() const
{
    Q_D(const WaylandSurface);
    if (!d->surface->extendedSurface())
        return QVariantMap();

    return d->surface->extendedSurface()->windowProperties();
}

void WaylandSurface::setWindowProperty(const QString &name, const QVariant &value)
{
    Q_D(WaylandSurface);
    if (!d->surface->extendedSurface())
        return;

    d->surface->extendedSurface()->setWindowProperty(name, value);
}

QPointF WaylandSurface::mapToParent(const QPointF &pos) const
{
    return pos + this->pos();
}

QPointF WaylandSurface::mapTo(WaylandSurface *parent, const QPointF &pos) const
{
    QPointF p = pos;
    if (parent) {
        const WaylandSurface * surface = this;
        while (surface != parent) {
            Q_ASSERT_X(surface, "WaylandSurface::mapTo(WaylandSurface *parent, const QPoint &pos)",
                       "parent must be in parent hierarchy");
            p = surface->mapToParent(p);
            surface = surface->parentSurface();
        }
    }
    return p;
}

WaylandCompositor *WaylandSurface::compositor() const
{
    Q_D(const WaylandSurface);
    return d->surface->compositor()->waylandCompositor();
}

void WaylandSurface::frameFinished()
{
    Q_D(WaylandSurface);
    d->surface->frameFinished();
}

WaylandSurface *WaylandSurface::transientParent() const
{
    Q_D(const WaylandSurface);
    if (d->surface->shellSurface() && d->surface->shellSurface()->transientParent())
        return d->surface->shellSurface()->transientParent()->surface()->waylandSurface();
    return 0;
}

void WaylandSurface::sendOnScreenVisibilityChange(bool visible)
{
    Q_D(WaylandSurface);
    if (d->surface->extendedSurface())
        d->surface->extendedSurface()->sendOnScreenVisibility(visible);
}

QString WaylandSurface::title() const
{
    Q_D(const WaylandSurface);
    return d->surface->title();
}

/*!
 * \return True if WL_SHELL_SURFACE_TRANSIENT_INACTIVE was set for this surface, meaning it should not receive keyboard focus.
 */
bool WaylandSurface::transientInactive() const
{
    Q_D(const WaylandSurface);
    return d->surface->transientInactive();
}
