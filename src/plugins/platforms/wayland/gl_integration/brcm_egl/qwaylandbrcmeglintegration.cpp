/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandbrcmeglintegration.h"

#include "gl_integration/qwaylandglintegration.h"

#include "qwaylandbrcmeglwindow.h"
#include "qwaylandbrcmglcontext.h"

#include <QtCore/QDebug>

#include "wayland-brcm-client-protocol.h"

QWaylandBrcmEglIntegration::QWaylandBrcmEglIntegration(struct wl_display *waylandDisplay)
    : m_waylandDisplay(waylandDisplay)
{
    wl_display_add_global_listener(waylandDisplay, wlDisplayHandleGlobal, this);
    qDebug() << "Using Brcm-EGL";
}

void QWaylandBrcmEglIntegration::wlDisplayHandleGlobal(wl_display *display, uint32_t id, const char *interface, uint32_t version, void *data)
{
    Q_UNUSED(version);
    if (strcmp(interface, "wl_brcm") == 0) {
        QWaylandBrcmEglIntegration *integration = static_cast<QWaylandBrcmEglIntegration *>(data);
        integration->m_waylandBrcm = static_cast<struct wl_brcm *>(wl_display_bind(display, id, &wl_brcm_interface));
    }
}

wl_brcm *QWaylandBrcmEglIntegration::waylandBrcm() const
{
    return m_waylandBrcm;
}

QWaylandBrcmEglIntegration::~QWaylandBrcmEglIntegration()
{
    eglTerminate(m_eglDisplay);
}

void QWaylandBrcmEglIntegration::initialize()
{
    EGLint major,minor;
    m_eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == NULL) {
        qWarning("EGL not available");
    } else {
        if (!eglInitialize(m_eglDisplay, &major, &minor)) {
            qWarning("failed to initialize EGL display");
            return;
        }

        eglFlushBRCM = (PFNEGLFLUSHBRCMPROC)eglGetProcAddress("eglFlushBRCM");
        if (!eglFlushBRCM) {
            qWarning("failed to resolve eglFlushBRCM, performance will suffer");
        }

        eglCreateGlobalImageBRCM = ::eglCreateGlobalImageBRCM;
        if (!eglCreateGlobalImageBRCM) {
            qWarning("failed to resolve eglCreateGlobalImageBRCM");
            return;
        }

        eglDestroyGlobalImageBRCM = ::eglDestroyGlobalImageBRCM;
        if (!eglDestroyGlobalImageBRCM) {
            qWarning("failed to resolve eglDestroyGlobalImageBRCM");
            return;
        }
    }
}

QWaylandWindow *QWaylandBrcmEglIntegration::createEglWindow(QWindow *window)
{
    return new QWaylandBrcmEglWindow(window);
}

QPlatformOpenGLContext *QWaylandBrcmEglIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const
{
    return new QWaylandBrcmGLContext(m_eglDisplay, glFormat, share);
}

EGLDisplay QWaylandBrcmEglIntegration::eglDisplay() const
{
    return m_eglDisplay;
}

QWaylandGLIntegration *QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandBrcmEglIntegration(waylandDisplay->wl_display());
}

