/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "xcompositehandler.h"
#include "xcompositebuffer.h"

QT_BEGIN_NAMESPACE

XCompositeBuffer::XCompositeBuffer(Window window, const QSize &size,
                                   struct ::wl_client *client, uint32_t id, XCompositeHandler *handler)
    : QtWaylandServer::wl_buffer(client, id, 1)
    , mWindow(window)
    , mOrigin(QWaylandSurface::OriginBottomLeft)
    , mSize(size)
    , mHandler(handler)
{
}

void XCompositeBuffer::buffer_destroy_resource(Resource *resource)
{
    mHandler->removeBuffer(resource->handle);
    delete this;
}

void XCompositeBuffer::buffer_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

Window XCompositeBuffer::window()
{
    return mWindow;
}

QT_END_NAMESPACE
