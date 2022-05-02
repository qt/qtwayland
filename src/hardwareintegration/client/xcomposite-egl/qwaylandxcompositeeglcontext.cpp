/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
**
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

#include "qwaylandxcompositeeglcontext.h"

#include "qwaylandxcompositeeglwindow.h"

#include <QtCore/QDebug>
#include <QtGui/QRegion>

#include <QtGui/private/qeglconvenience_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXCompositeEGLContext::QWaylandXCompositeEGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display, EGLConfig config)
    : QEGLPlatformContext(format, share, display, &config)
{
}

void QWaylandXCompositeEGLContext::swapBuffers(QPlatformSurface *surface)
{
    QEGLPlatformContext::swapBuffers(surface);

    QWaylandXCompositeEGLWindow *w =
        static_cast<QWaylandXCompositeEGLWindow *>(surface);

    QSize size = w->geometry().size();

    w->commit(w->buffer(), QRegion(0, 0, size.width(), size.height()));
    w->waitForFrameSync(100);
}

EGLSurface QWaylandXCompositeEGLContext::eglSurfaceForPlatformSurface(QPlatformSurface *surface)
{
    return static_cast<QWaylandXCompositeEGLWindow *>(surface)->eglSurface();
}

}

QT_END_NAMESPACE
