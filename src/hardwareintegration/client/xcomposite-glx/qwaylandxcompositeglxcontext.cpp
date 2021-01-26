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

#include <QtCore/QDebug>

#include "qwaylandxcompositeglxcontext.h"

#include "qwaylandxcompositeglxwindow.h"

#include <QRegion>

#include <dlfcn.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXCompositeGLXContext::QWaylandXCompositeGLXContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, Display *display, int screen)
    : m_display(display),
      m_format(format)
{
    qDebug("creating XComposite-GLX context");

    if (m_format.renderableType() == QSurfaceFormat::DefaultRenderableType)
        m_format.setRenderableType(QSurfaceFormat::OpenGL);

    if (m_format.renderableType() != QSurfaceFormat::OpenGL) {
        qWarning("Unsupported renderable type");
        return;
    }

    GLXContext shareContext = share ? static_cast<QWaylandXCompositeGLXContext *>(share)->m_context : 0;
    GLXFBConfig config = qglx_findConfig(display, screen, m_format, GLX_WINDOW_BIT | GLX_PIXMAP_BIT);
    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(display, config);
    m_context = glXCreateContext(display, visualInfo, shareContext, true);
    qglx_surfaceFormatFromGLXFBConfig(&m_format, display, config);
}

bool QWaylandXCompositeGLXContext::makeCurrent(QPlatformSurface *surface)
{
    Window xWindow = static_cast<QWaylandXCompositeGLXWindow *>(surface)->xWindow();

    return glXMakeCurrent(m_display, xWindow, m_context);
}

void QWaylandXCompositeGLXContext::doneCurrent()
{
    glXMakeCurrent(m_display, 0, 0);
}

void QWaylandXCompositeGLXContext::swapBuffers(QPlatformSurface *surface)
{
    QWaylandXCompositeGLXWindow *w = static_cast<QWaylandXCompositeGLXWindow *>(surface);

    QSize size = w->geometry().size();

    glXSwapBuffers(m_display, w->xWindow());

    w->commit(w->buffer(), QRegion(0, 0, size.width(), size.height()));
    w->waitForFrameSync(100);
}

QFunctionPointer QWaylandXCompositeGLXContext::getProcAddress(const char *procName)
{
    QFunctionPointer proc = glXGetProcAddress(reinterpret_cast<const GLubyte *>(procName));
    if (!proc)
        proc = (QFunctionPointer) dlsym(RTLD_DEFAULT, procName);
    return proc;
}

QSurfaceFormat QWaylandXCompositeGLXContext::format() const
{
    return m_format;
}

}

QT_END_NAMESPACE
