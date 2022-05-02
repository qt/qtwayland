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

#ifndef QWAYLANDGLCONTEXT_H
#define QWAYLANDGLCONTEXT_H

#include "qwaylandeglinclude_p.h" //must be first

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtGui/private/qeglplatformcontext_p.h>
#include <qpa/qplatformopenglcontext.h>

QT_BEGIN_NAMESPACE

class QOpenGLShaderProgram;
class QOpenGLTextureCache;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandGLWindowSurface;
class DecorationsBlitter;

class Q_WAYLAND_CLIENT_EXPORT QWaylandGLContext : public QEGLPlatformContext
{
public:
    QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display, const QSurfaceFormat &format, QPlatformOpenGLContext *share);
    ~QWaylandGLContext();
    void swapBuffers(QPlatformSurface *surface) override;

    bool makeCurrent(QPlatformSurface *surface) override;
    void doneCurrent() override;

    GLuint defaultFramebufferObject(QPlatformSurface *surface) const override;

    QFunctionPointer getProcAddress(const char *procName) override;

protected:
    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) override;
    EGLSurface createTemporaryOffscreenSurface() override;
    void destroyTemporaryOffscreenSurface(EGLSurface surface) override;

private:
    QWaylandDisplay *m_display = nullptr;
    EGLContext m_decorationsContext;
    DecorationsBlitter *m_blitter = nullptr;
    bool m_supportNonBlockingSwap = true;
    EGLenum m_api;
    wl_surface *m_wlSurface = nullptr;
    wl_egl_window *m_eglWindow = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDGLCONTEXT_H
