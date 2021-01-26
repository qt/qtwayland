/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef WAYLANDEGLSTREAMCONTROLLER_H
#define WAYLANDEGLSTREAMCONTROLLER_H

#include "qwayland-server-wl-eglstream-controller.h"

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QSize>
#include <QtCore/QTextStream>
#include <QtGui/QOpenGLTexture>

#include <EGL/egl.h>
#include <EGL/eglext.h>


QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandResource;
class WaylandEglStreamClientBufferIntegration;

class WaylandEglStreamController : public QtWaylandServer::wl_eglstream_controller
{
public:
    explicit WaylandEglStreamController(wl_display *display, WaylandEglStreamClientBufferIntegration *clientBufferIntegration);

protected:
    void eglstream_controller_attach_eglstream_consumer(Resource *resource, struct ::wl_resource *wl_surface, struct ::wl_resource *wl_buffer) override;

private:
    WaylandEglStreamClientBufferIntegration *m_clientBufferIntegration;
};


QT_END_NAMESPACE

#endif // WAYLANDEGLSTREAMCONTROLLER_H
