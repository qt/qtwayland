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

#include <QtCore/qglobal.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QWindow;
class QPlatformOpenGLContext;
class QSurfaceFormat;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientBufferIntegration
{
public:
    QWaylandClientBufferIntegration();
    virtual ~QWaylandClientBufferIntegration();

    virtual void initialize(QWaylandDisplay *display) = 0;

    virtual bool isValid() const { return true; }

    virtual bool supportsThreadedOpenGL() const { return false; }
    virtual bool supportsWindowDecoration() const { return false; }

    virtual QWaylandWindow *createEglWindow(QWindow *window) = 0;
    virtual QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const = 0;

    enum NativeResource {
        EglDisplay,
        EglConfig,
        EglContext
    };
    virtual void *nativeResource(NativeResource /*resource*/) { return nullptr; }
    virtual void *nativeResourceForContext(NativeResource /*resource*/, QPlatformOpenGLContext */*context*/) { return nullptr; }
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTBUFFERINTEGRATION_H
