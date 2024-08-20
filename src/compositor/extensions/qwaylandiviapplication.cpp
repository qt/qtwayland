// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandiviapplication.h"
#include "qwaylandiviapplication_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandIviSurface>
#include <QtWaylandCompositor/QWaylandResource>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype IviApplication
 * \nativetype QWaylandIviApplication
 * \inqmlmodule QtWayland.Compositor.IviApplication
 * \since 5.8
 * \brief Provides a shell extension for embedded-style user interfaces.
 *
 * The IviApplication extension provides a way to associate an IviSurface
 * with a regular Wayland surface. Using the IviSurface interface, the client can identify
 * itself by giving an ivi id, and the compositor can ask the client to resize.
 *
 * IviApplication corresponds to the Wayland \c ivi_application interface.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the IviApplication component and add it to the list of extensions
 * supported by the compositor:
 *
 * \qml
 * import QtWayland.Compositor.IviApplication
 *
 * WaylandCompositor {
 *     IviApplication {
 *         onIviSurfaceCreated: {
 *             if (iviSurface.iviId === navigationIviId) {
 *                 // ...
 *             }
 *         }
 *     }
 * }
 * \endqml
 */

/*!
 * \class QWaylandIviApplication
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandIviApplication class is an extension for embedded-style user interfaces.
 *
 * The QWaylandIviApplication extension provides a way to associate an QWaylandIviSurface
 * with a regular Wayland surface. Using the QWaylandIviSurface interface, the client can identify
 * itself by giving an ivi id, and the compositor can ask the client to resize.
 *
 * QWaylandIviApplication corresponds to the Wayland \c ivi_application interface.
 */

/*!
 * Constructs a QWaylandIviApplication object.
 */
QWaylandIviApplication::QWaylandIviApplication()
    : QWaylandCompositorExtensionTemplate<QWaylandIviApplication>(*new QWaylandIviApplicationPrivate())
{
}

/*!
 * Constructs a QWaylandIviApplication object for the provided \a compositor.
 */
QWaylandIviApplication::QWaylandIviApplication(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandIviApplication>(compositor, *new QWaylandIviApplicationPrivate())
{
}

/*!
 * Initializes the shell extension.
 */
void QWaylandIviApplication::initialize()
{
    Q_D(QWaylandIviApplication);
    QWaylandCompositorExtensionTemplate::initialize();

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandIviApplication";
        return;
    }

    d->init(compositor->display(), 1);
}

/*!
 * Returns the Wayland interface for the QWaylandIviApplication.
 */
const struct wl_interface *QWaylandIviApplication::interface()
{
    return QWaylandIviApplicationPrivate::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandIviApplication::interfaceName()
{
    return QWaylandIviApplicationPrivate::interfaceName();
}

/*!
 * \qmlsignal void IviApplication::iviSurfaceRequested(WaylandSurface surface, int iviId, WaylandResource resource)
 *
 * This signal is emitted when the client has requested an \c ivi_surface to be associated
 * with \a surface, which is identified by \a iviId. The handler for this signal is
 * expected to create the ivi surface for \a resource and initialize it within the scope of the
 * signal emission. If no ivi surface is created, a default one will be created instead.
 *
 */

/*!
 * \fn void QWaylandIviApplication::iviSurfaceRequested(QWaylandSurface *surface, uint iviId, const QWaylandResource &resource)
 *
 * This signal is emitted when the client has requested an \c ivi_surface to be associated
 * with \a surface, which is identified by \a iviId. The handler for this signal is
 * expected to create the ivi surface for \a resource and initialize it within the scope of the
 * signal emission. If no ivi surface is created, a default one will be created instead.
 */

/*!
 * \qmlsignal void IviApplication::iviSurfaceCreated(IviSurface *iviSurface)
 *
 * This signal is emitted when an IviSurface has been created. The supplied \a iviSurface is
 * most commonly used to instantiate a ShellSurfaceItem.
 */

/*!
 * \fn void QWaylandIviApplication::iviSurfaceCreated(QWaylandIviSurface *iviSurface)
 *
 * This signal is emitted when an IviSurface, \a iviSurface, has been created.
 */

QWaylandIviApplicationPrivate::QWaylandIviApplicationPrivate()
{
}

void QWaylandIviApplicationPrivate::unregisterIviSurface(QWaylandIviSurface *iviSurface)
{
    m_iviSurfaces.remove(iviSurface->iviId());
}

void QWaylandIviApplicationPrivate::ivi_application_surface_create(QtWaylandServer::ivi_application::Resource *resource,
                                                                   uint32_t ivi_id, wl_resource *surfaceResource, uint32_t id)
{
    Q_Q(QWaylandIviApplication);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    if (m_iviSurfaces.contains(ivi_id)) {
        wl_resource_post_error(resource->handle, IVI_APPLICATION_ERROR_IVI_ID,
                               "Given ivi_id, %d, is already assigned to wl_surface@%d", ivi_id,
                               wl_resource_get_id(m_iviSurfaces[ivi_id]->surface()->resource()));
        return;
    }

    if (!surface->setRole(QWaylandIviSurface::role(), resource->handle, IVI_APPLICATION_ERROR_ROLE))
        return;

    QWaylandResource iviSurfaceResource(wl_resource_create(resource->client(), &ivi_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    emit q->iviSurfaceRequested(surface, ivi_id, iviSurfaceResource);

    QWaylandIviSurface *iviSurface = QWaylandIviSurface::fromResource(iviSurfaceResource.resource());

    if (!iviSurface)
        iviSurface = new QWaylandIviSurface(q, surface, ivi_id, iviSurfaceResource);

    m_iviSurfaces.insert(ivi_id, iviSurface);

    emit q->iviSurfaceCreated(iviSurface);
}

QT_END_NAMESPACE

#include "moc_qwaylandiviapplication.cpp"
