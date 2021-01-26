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
****************************************************************************/

#ifndef QWAYLANDXCOMPOSITEEGLCONTEXT_H
#define QWAYLANDXCOMPOSITEEGLCONTEXT_H

#include <qpa/qplatformopenglcontext.h>

#include "qwaylandxcompositeeglclientbufferintegration.h"

#include <QtEglSupport/private/qeglplatformcontext_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandXCompositeEGLWindow;

class QWaylandXCompositeEGLContext : public QEGLPlatformContext
{
public:
    QWaylandXCompositeEGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display, EGLConfig config);

    void swapBuffers(QPlatformSurface *surface) override;

private:
    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) override;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXCOMPOSITEEGLCONTEXT_H
