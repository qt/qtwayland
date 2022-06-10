// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandsurfacegrabber.h"

#include <QtCore/private/qobject_p.h>
#include <QtWaylandCompositor/qwaylandsurface.h>
#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QWaylandSurfaceGrabber
    \inmodule QtWaylandCompositor
    \since 5.8
    \brief The QWaylandSurfaceGrabber class allows to read the content of a QWaylandSurface.

    Sometimes it is needed to get the contents of a surface, for example to provide a screenshot
    to the user. The QWaylandSurfaceGrabber class provides a simple method to do so, without
    having to care what type of buffer backs the surface, be it shared memory, OpenGL or something
    else.
*/

/*!
    \enum QWaylandSurfaceGrabber::Error

    The Error enum describes the reason for a grab failure.

    \value InvalidSurface The surface is null or otherwise not valid.
    \value NoBufferAttached The client has not attached a buffer on the surface yet.
    \value UnknownBufferType The buffer attached on the surface is of an unknown type.
    \value RendererNotReady The compositor renderer is not ready to grab the surface content.
 */

class QWaylandSurfaceGrabberPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandSurfaceGrabber)

    QWaylandSurface *surface = nullptr;
};

/*!
 * Create a QWaylandSurfaceGrabber object with the given \a surface and \a parent
 */
QWaylandSurfaceGrabber::QWaylandSurfaceGrabber(QWaylandSurface *surface, QObject *parent)
                      : QObject(*(new QWaylandSurfaceGrabberPrivate), parent)
{
    Q_D(QWaylandSurfaceGrabber);
    d->surface = surface;
}

/*!
 * Returns the surface set on this object
 */
QWaylandSurface *QWaylandSurfaceGrabber::surface() const
{
    Q_D(const QWaylandSurfaceGrabber);
    return d->surface;
}

/*!
 * Grab the content of the surface set on this object.
 * It may not be possible to do that immediately so the success and failed signals
 * should be used to be notified of when the grab is completed.
 */
void QWaylandSurfaceGrabber::grab()
{
    Q_D(QWaylandSurfaceGrabber);
    if (!d->surface) {
        emit failed(InvalidSurface);
        return;
    }

    QWaylandSurfacePrivate *surf = QWaylandSurfacePrivate::get(d->surface);
    QWaylandBufferRef buf = surf->bufferRef;
    if (!buf.hasBuffer()) {
        emit failed(NoBufferAttached);
        return;
    }

    d->surface->compositor()->grabSurface(this, buf);
}

QT_END_NAMESPACE

#include "moc_qwaylandsurfacegrabber.cpp"
