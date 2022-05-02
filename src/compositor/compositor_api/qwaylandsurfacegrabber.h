/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QWAYLANDSURFACEGRABBER_H
#define QWAYLANDSURFACEGRABBER_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandSurfaceGrabberPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandSurfaceGrabber : public QObject
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
