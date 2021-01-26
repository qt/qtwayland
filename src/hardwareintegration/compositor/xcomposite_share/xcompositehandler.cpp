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

#include "xcompositehandler.h"

#include "wayland-xcomposite-server-protocol.h"

#include "xcompositebuffer.h"
#include <X11/extensions/Xcomposite.h>

QT_BEGIN_NAMESPACE

XCompositeHandler::XCompositeHandler(QWaylandCompositor *compositor, Display *display)
    : QtWaylandServer::qt_xcomposite(compositor->display(), 1)
{
    mFakeRootWindow = new QWindow();
    mFakeRootWindow->setGeometry(QRect(-1,-1,1,1));
    mFakeRootWindow->create();
    mFakeRootWindow->show();

    int composite_event_base, composite_error_base;
    if (!XCompositeQueryExtension(display, &composite_event_base, &composite_error_base))
        qFatal("XComposite required");

    mDisplayString = QString::fromLocal8Bit(XDisplayString(display));
}

void XCompositeHandler::xcomposite_bind_resource(Resource *resource)
{
    send_root(resource->handle, mDisplayString, mFakeRootWindow->winId());
}

void XCompositeHandler::xcomposite_create_buffer(Resource *resource, uint32_t id, uint32_t window,
                                                 int32_t width, int32_t height)
{
    new XCompositeBuffer(Window(window), QSize(width, height), resource->client(), id);
}

QT_END_NAMESPACE
