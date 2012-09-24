/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandglcontext.h"

#include "qwaylanddisplay.h"
#include "qwaylandwindow.h"
#include "qwaylandeglwindow.h"

#include <QtPlatformSupport/private/qeglconvenience_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_config(q_configFromGLFormat(m_eglDisplay, format, true))
    , m_format(q_glFormatFromConfig(m_eglDisplay, m_config))
{
    m_shareEGLContext = share ? static_cast<QWaylandGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;

    eglBindAPI(EGL_OPENGL_ES_API);

    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(format.majorVersion() == 1 ? 1 : 2);
    eglContextAttrs.append(EGL_NONE);

    m_context = eglCreateContext(m_eglDisplay, m_config, m_shareEGLContext, eglContextAttrs.constData());

    if (m_context == EGL_NO_CONTEXT) {
        m_context = eglCreateContext(m_eglDisplay, m_config, EGL_NO_CONTEXT, eglContextAttrs.constData());
        m_shareEGLContext = EGL_NO_CONTEXT;
    }
}

QWaylandGLContext::~QWaylandGLContext()
{
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
    EGLSurface eglSurface = static_cast<QWaylandEglWindow *>(surface)->eglSurface();
    return eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context);
}

void QWaylandGLContext::doneCurrent()
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void QWaylandGLContext::swapBuffers(QPlatformSurface *surface)
{
    EGLSurface eglSurface = static_cast<QWaylandEglWindow *>(surface)->eglSurface();
    eglSwapBuffers(m_eglDisplay, eglSurface);
}

bool QWaylandGLContext::isSharing() const
{
    return m_shareEGLContext != EGL_NO_CONTEXT;
}

bool QWaylandGLContext::isValid() const
{
    return m_context != EGL_NO_CONTEXT;
}

void (*QWaylandGLContext::getProcAddress(const QByteArray &procName)) ()
{
    return eglGetProcAddress(procName.constData());
}

EGLConfig QWaylandGLContext::eglConfig() const
{
    return m_config;
}

