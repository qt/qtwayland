// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCLIENTBUFFERINTEGRATION_H
#define QWAYLANDCLIENTBUFFERINTEGRATION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandsurface.h>
#include <QtWaylandCompositor/qwaylandbufferref.h>
#include <QtCore/QSize>
#include <QtCore/private/qglobal_p.h>
#include <wayland-server-core.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QOpenGLTexture;

namespace QtWayland {
class Display;

class Q_WAYLANDCOMPOSITOR_EXPORT ClientBufferIntegration
{
public:
    ClientBufferIntegration();
    virtual ~ClientBufferIntegration() { }

    void setCompositor(QWaylandCompositor *compositor) { m_compositor = compositor; }
    QWaylandCompositor *compositor() const { return m_compositor; }

    virtual void initializeHardware(struct ::wl_display *display) = 0;

    virtual ClientBuffer *createBufferFor(struct ::wl_resource *buffer) = 0;
    virtual bool isProtected(struct ::wl_resource *buffer) { Q_UNUSED(buffer); return false; }

protected:
    QWaylandCompositor *m_compositor = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTBUFFERINTEGRATION_H
