/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef XCOMPOSITEBUFFER_H
#define XCOMPOSITEBUFFER_H

#include <qwayland-server-wayland.h>

#include <QtWaylandCompositor/private/qwaylandutils_p.h>

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtCore/QSize>

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>

#include <X11/X.h>

QT_BEGIN_NAMESPACE

class XCompositeBuffer : public QtWaylandServer::wl_buffer
{
public:
    XCompositeBuffer(Window window, const QSize &size,
                     struct ::wl_client *client, uint32_t id);

    Window window();

    QWaylandSurface::Origin origin() const { return mOrigin; }
    void setOrigin(QWaylandSurface::Origin origin) { mOrigin = origin; }

    QSize size() const { return mSize; }

    static XCompositeBuffer *fromResource(struct ::wl_resource *resource) { return QtWayland::fromResource<XCompositeBuffer *>(resource); }

protected:
    void buffer_destroy_resource(Resource *) override;
    void buffer_destroy(Resource *) override;

private:
    Window mWindow;
    QWaylandSurface::Origin mOrigin;
    QSize mSize;
};

QT_END_NAMESPACE

#endif // XCOMPOSITORBUFFER_H
