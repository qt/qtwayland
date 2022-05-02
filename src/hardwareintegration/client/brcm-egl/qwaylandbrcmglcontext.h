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

#ifndef QWAYLANDBRCMGLCONTEXT_H
#define QWAYLANDBRCMGLCONTEXT_H

#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#include <qpa/qplatformopenglcontext.h>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandGLWindowSurface;

class QWaylandBrcmGLContext : public QPlatformOpenGLContext {
public:
    QWaylandBrcmGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share);
    ~QWaylandBrcmGLContext();

    void swapBuffers(QPlatformSurface *surface) override;

    bool makeCurrent(QPlatformSurface *surface) override;
    void doneCurrent() override;

    QFunctionPointer getProcAddress(const char *procName) override;

    QSurfaceFormat format() const override { return m_format; }

    EGLConfig eglConfig() const;
    EGLContext eglContext() const { return m_context; }

private:
    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;

    EGLContext m_context;
    EGLConfig m_config;
    QSurfaceFormat m_format;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDBRCMGLCONTEXT_H
