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

#ifndef QWAYLANDEGLINTEGRATION_H
#define QWAYLANDEGLINTEGRATION_H

#include <QtWaylandClient/private/qwaylandclientbufferintegration_p.h>

#include "qwaylandeglinclude.h"

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;

class QWaylandEglClientBufferIntegration : public QWaylandClientBufferIntegration
{
public:
    QWaylandEglClientBufferIntegration();
    ~QWaylandEglClientBufferIntegration() override;

    void initialize(QWaylandDisplay *display) override;
    bool isValid() const override;
    bool supportsThreadedOpenGL() const override;
    bool supportsWindowDecoration() const override;

    QWaylandWindow *createEglWindow(QWindow *window) override;
    QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const override;

    void *nativeResource(NativeResource resource) override;
    void *nativeResourceForContext(NativeResource resource, QPlatformOpenGLContext *context) override;

    EGLDisplay eglDisplay() const;

private:
    QWaylandDisplay *m_display = nullptr;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    bool m_supportsThreading = false;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDEGLINTEGRATION_H
