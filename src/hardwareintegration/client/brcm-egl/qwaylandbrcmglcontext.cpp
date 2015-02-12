/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandbrcmglcontext.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include "qwaylandbrcmeglwindow.h"

#include <QtPlatformSupport/private/qeglconvenience_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

extern QSurfaceFormat brcmFixFormat(const QSurfaceFormat &format);

QWaylandBrcmGLContext::QWaylandBrcmGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_config(q_configFromGLFormat(m_eglDisplay, brcmFixFormat(format), true))
    , m_format(q_glFormatFromConfig(m_eglDisplay, m_config))
{
    EGLContext shareEGLContext = share ? static_cast<QWaylandBrcmGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;

    eglBindAPI(EGL_OPENGL_ES_API);

    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(format.majorVersion() == 1 ? 1 : 2);
    eglContextAttrs.append(EGL_NONE);

    m_context = eglCreateContext(m_eglDisplay, m_config, shareEGLContext, eglContextAttrs.constData());
}

QWaylandBrcmGLContext::~QWaylandBrcmGLContext()
{
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandBrcmGLContext::makeCurrent(QPlatformSurface *surface)
{
    return static_cast<QWaylandBrcmEglWindow *>(surface)->makeCurrent(m_context);
}

void QWaylandBrcmGLContext::doneCurrent()
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void QWaylandBrcmGLContext::swapBuffers(QPlatformSurface *surface)
{
    static_cast<QWaylandBrcmEglWindow *>(surface)->swapBuffers();
}

void (*QWaylandBrcmGLContext::getProcAddress(const QByteArray &procName)) ()
{
    return eglGetProcAddress(procName.constData());
}

EGLConfig QWaylandBrcmGLContext::eglConfig() const
{
    return m_config;
}

}

QT_END_NAMESPACE
