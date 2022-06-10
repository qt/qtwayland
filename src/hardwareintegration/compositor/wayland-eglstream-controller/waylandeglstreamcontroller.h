// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WAYLANDEGLSTREAMCONTROLLER_H
#define WAYLANDEGLSTREAMCONTROLLER_H

#include "qwayland-server-wl-eglstream-controller.h"

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>

#include <QtOpenGL/QOpenGLTexture>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QSize>
#include <QtCore/QTextStream>

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
