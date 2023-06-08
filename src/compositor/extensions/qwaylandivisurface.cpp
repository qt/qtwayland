// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandivisurface.h"
#include "qwaylandivisurface_p.h"
#include "qwaylandiviapplication_p.h"
#if QT_CONFIG(wayland_compositor_quick)
#include "qwaylandivisurfaceintegration_p.h"
#endif

#include <QtWaylandCompositor/QWaylandResource>
#include <QDebug>

#include <QtWaylandCompositor/private/qwaylandutils_p.h>

QT_BEGIN_NAMESPACE

QWaylandSurfaceRole QWaylandIviSurfacePrivate::s_role("ivi_surface");

/*!
 * \qmltype IviSurface
 * \instantiates QWaylandIviSurface
 * \inqmlmodule QtWayland.Compositor.IviApplication
 * \since 5.8
 * \brief Provides a simple way to identify and resize a surface.
 *
 * This type is part of the \l{IviApplication} extension and provides a way to extend
 * the functionality of an existing WaylandSurface with a way to resize and identify it.
 *
 * It corresponds to the Wayland \c ivi_surface interface.
 */

/*!
 * \class QWaylandIviSurface
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandIviSurface class provides a simple way to identify and resize a surface.
 *
 * This class is part of the QWaylandIviApplication extension and provides a way to
 * extend the functionality of an existing QWaylandSurface with a way to resize and identify it.
 *
 * It corresponds to the Wayland \c ivi_surface interface.
 */

/*!
 * Constructs a QWaylandIviSurface.
 */
QWaylandIviSurface::QWaylandIviSurface()
    : QWaylandShellSurfaceTemplate<QWaylandIviSurface>(*new QWaylandIviSurfacePrivate())
{
}

/*!
 * Constructs a QWaylandIviSurface for \a surface and initializes it with the
 * given \a application, \a surface, \a iviId, and \a resource.
 */
QWaylandIviSurface::QWaylandIviSurface(QWaylandIviApplication *application, QWaylandSurface *surface, uint iviId, const QWaylandResource &resource)
    : QWaylandShellSurfaceTemplate<QWaylandIviSurface>(*new QWaylandIviSurfacePrivate())
{
    initialize(application, surface, iviId, resource);
}

/*!
 * \qmlmethod void IviSurface::initialize(IviApplication iviApplication, WaylandSurface surface, int iviId, WaylandResource resource)
 *
 * Initializes the IviSurface, associating it with the given \a iviApplication, \a surface,
 * \a iviId, and \a resource.
 */

/*!
 * Initializes the QWaylandIviSurface, associating it with the given \a iviApplication, \a surface,
 * \a iviId, and \a resource.
 */
void QWaylandIviSurface::initialize(QWaylandIviApplication *iviApplication, QWaylandSurface *surface, uint iviId, const QWaylandResource &resource)
{
    Q_D(QWaylandIviSurface);

    d->m_iviApplication = iviApplication;
    d->m_surface = surface;
    d->m_iviId = iviId;

    d->init(resource.resource());
    setExtensionContainer(surface);

    emit surfaceChanged();
    emit iviIdChanged();

    QWaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty WaylandSurface IviSurface::surface
 *
 * This property holds the surface associated with this IviSurface.
 */

/*!
 * \property QWaylandIviSurface::surface
 *
 * This property holds the surface associated with this QWaylandIviSurface.
 */
QWaylandSurface *QWaylandIviSurface::surface() const
{
    Q_D(const QWaylandIviSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty int IviSurface::iviId
 * \readonly
 *
 * This property holds the ivi id id of this IviSurface.
 */

/*!
 * \property QWaylandIviSurface::iviId
 *
 * This property holds the ivi id of this QWaylandIviSurface.
 */
uint QWaylandIviSurface::iviId() const
{
    Q_D(const QWaylandIviSurface);
    return d->m_iviId;
}

/*!
 * Returns the Wayland interface for the QWaylandIviSurface.
 */
const struct wl_interface *QWaylandIviSurface::interface()
{
    return QWaylandIviSurfacePrivate::interface();
}

QByteArray QWaylandIviSurface::interfaceName()
{
    return QWaylandIviSurfacePrivate::interfaceName();
}

/*!
 * Returns the surface role for the QWaylandIviSurface.
 */
QWaylandSurfaceRole *QWaylandIviSurface::role()
{
    return &QWaylandIviSurfacePrivate::s_role;
}

/*!
 * Returns the QWaylandIviSurface corresponding to the \a resource.
 */
QWaylandIviSurface *QWaylandIviSurface::fromResource(wl_resource *resource)
{
    if (auto p = QtWayland::fromResource<QWaylandIviSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * \qmlmethod int IviSurface::sendConfigure(size size)
 *
 * Sends a configure event to the client, telling it to resize the surface to the given \a size.
 */

/*!
 * Sends a configure event to the client, telling it to resize the surface to the given \a size.
 */
void QWaylandIviSurface::sendConfigure(const QSize &size)
{
    if (!size.isValid()) {
        qWarning() << "Can't configure ivi_surface with an invalid size" << size;
        return;
    }
    Q_D(QWaylandIviSurface);
    d->send_configure(size.width(), size.height());
}

#if QT_CONFIG(wayland_compositor_quick)
QWaylandQuickShellIntegration *QWaylandIviSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new QtWayland::IviSurfaceIntegration(item);
}
#endif

/*!
 * \internal
 */
void QWaylandIviSurface::initialize()
{
    QWaylandShellSurfaceTemplate::initialize();
}

QWaylandIviSurfacePrivate::QWaylandIviSurfacePrivate()
{
}

void QWaylandIviSurfacePrivate::ivi_surface_destroy_resource(QtWaylandServer::ivi_surface::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandIviSurface);
    QWaylandIviApplicationPrivate::get(m_iviApplication)->unregisterIviSurface(q);
    delete q;
}

void QWaylandIviSurfacePrivate::ivi_surface_destroy(QtWaylandServer::ivi_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

QT_END_NAMESPACE

#include "moc_qwaylandivisurface.cpp"
