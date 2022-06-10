// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSURFACEGRABBER_H
#define QWAYLANDSURFACEGRABBER_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandSurfaceGrabberPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandSurfaceGrabber : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurfaceGrabber)
public:
    enum Error {
        InvalidSurface,
        NoBufferAttached,
        UnknownBufferType,
        RendererNotReady,
    };
    Q_ENUM(Error)
    explicit QWaylandSurfaceGrabber(QWaylandSurface *surface, QObject *parent = nullptr);

    QWaylandSurface *surface() const;
    void grab();

Q_SIGNALS:
    void success(const QImage &image);
    void failed(Error error);
};

QT_END_NAMESPACE

#endif // QWAYLANDSURFACEGRABBER_H
