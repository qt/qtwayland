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

#ifndef QWAYLANDGLCONTEXT_H
#define QWAYLANDGLCONTEXT_H

#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#include <qpa/qplatformopenglcontext.h>

#include "qwaylandeglinclude.h"

QT_BEGIN_NAMESPACE

class QOpenGLShaderProgram;
class QOpenGLTextureCache;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandGLWindowSurface;
class DecorationsBlitter;

class QWaylandGLContext : public QPlatformOpenGLContext
{
public:
    QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display, const QSurfaceFormat &format, QPlatformOpenGLContext *share);
    ~QWaylandGLContext();

    void swapBuffers(QPlatformSurface *surface) override;

    bool makeCurrent(QPlatformSurface *surface) override;
    void doneCurrent() override;

    GLuint defaultFramebufferObject(QPlatformSurface *surface) const override;

    bool isSharing() const override;
    bool isValid() const override;

    QFunctionPointer getProcAddress(const char *procName) override;

    QSurfaceFormat format() const override { return m_format; }

    EGLConfig eglConfig() const;
    EGLContext eglContext() const { return m_context; }

private:
    void updateGLFormat();

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    QWaylandDisplay *m_display = nullptr;
    EGLContext m_context;
    EGLContext m_shareEGLContext;
    EGLContext m_decorationsContext;
    EGLConfig m_config;
    QSurfaceFormat m_format;
    DecorationsBlitter *m_blitter = nullptr;
    uint m_api;
    bool m_supportNonBlockingSwap = true;
    bool m_supportSurfaceLessContext = false;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDGLCONTEXT_H
