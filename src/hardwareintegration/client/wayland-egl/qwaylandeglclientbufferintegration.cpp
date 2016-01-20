/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandeglclientbufferintegration.h"

#include "qwaylandeglwindow.h"
#include "qwaylandglcontext.h"

#include <wayland-client.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

static const char *qwaylandegl_threadedgl_blacklist_vendor[] = {
    0
};

QWaylandEglClientBufferIntegration::QWaylandEglClientBufferIntegration()
    : m_display(0)
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_supportsThreading(false)
{
    qDebug() << "Using Wayland-EGL";
}


QWaylandEglClientBufferIntegration::~QWaylandEglClientBufferIntegration()
{
    eglTerminate(m_eglDisplay);
}

void QWaylandEglClientBufferIntegration::initialize(QWaylandDisplay *display)
{
    QByteArray eglPlatform = qgetenv("EGL_PLATFORM");
    if (eglPlatform.isEmpty()) {
        setenv("EGL_PLATFORM","wayland",true);
    }

    m_display = display;

    EGLint major,minor;
    m_eglDisplay = eglGetDisplay((EGLNativeDisplayType) display->wl_display());
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qWarning("EGL not available");
        return;
    }

    if (!eglInitialize(m_eglDisplay, &major, &minor)) {
        qWarning("failed to initialize EGL display");
        m_eglDisplay = EGL_NO_DISPLAY;
        return;
    }

    m_supportsThreading = true;
    if (qEnvironmentVariableIsSet("QT_OPENGL_NO_SANITY_CHECK"))
        return;

    const char *vendor = eglQueryString(m_eglDisplay, EGL_VENDOR);
    for (int i = 0; qwaylandegl_threadedgl_blacklist_vendor[i]; ++i) {
        if (strstr(vendor, qwaylandegl_threadedgl_blacklist_vendor[i]) != 0) {
            m_supportsThreading = false;
            break;
        }
    }
}

bool QWaylandEglClientBufferIntegration::isValid() const
{
    return m_eglDisplay != EGL_NO_DISPLAY;
}

bool QWaylandEglClientBufferIntegration::supportsThreadedOpenGL() const
{
    return m_supportsThreading;
}

bool QWaylandEglClientBufferIntegration::supportsWindowDecoration() const
{
    return true;
}

QWaylandWindow *QWaylandEglClientBufferIntegration::createEglWindow(QWindow *window)
{
    return new QWaylandEglWindow(window);
}

QPlatformOpenGLContext *QWaylandEglClientBufferIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const
{
    return new QWaylandGLContext(m_eglDisplay, m_display, glFormat, share);
}

void *QWaylandEglClientBufferIntegration::nativeResource(NativeResource resource)
{
    switch (resource) {
    case EglDisplay:
        return m_eglDisplay;
    default:
        break;
    }
    return Q_NULLPTR;
}

void *QWaylandEglClientBufferIntegration::nativeResourceForContext(NativeResource resource, QPlatformOpenGLContext *context)
{
    Q_ASSERT(context);
    switch (resource) {
    case EglConfig:
        return static_cast<QWaylandGLContext *>(context)->eglConfig();
    case EglContext:
        return static_cast<QWaylandGLContext *>(context)->eglContext();
    case EglDisplay:
        return m_eglDisplay;
    default:
        break;
    }
    return Q_NULLPTR;
}

EGLDisplay QWaylandEglClientBufferIntegration::eglDisplay() const
{
    return m_eglDisplay;
}

}

QT_END_NAMESPACE
