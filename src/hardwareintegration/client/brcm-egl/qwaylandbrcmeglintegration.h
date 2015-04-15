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

#ifndef QWAYLANDBRCMEGLINTEGRATION_H
#define QWAYLANDBRCMEGLINTEGRATION_H

#include <QtWaylandClient/private/qwaylandclientbufferintegration_p.h>
#include <wayland-client.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext_brcm.h>

#include <QtCore/qglobal.h>

struct qt_brcm;

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;

class QWaylandBrcmEglIntegration : public QWaylandClientBufferIntegration
{
public:
    QWaylandBrcmEglIntegration();
    ~QWaylandBrcmEglIntegration();

    void initialize(QWaylandDisplay *waylandDisplay);

    bool supportsThreadedOpenGL() const { return true; }
    bool supportsWindowDecoration() const { return false; }

    QWaylandWindow *createEglWindow(QWindow *window);
    QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const;

    EGLDisplay eglDisplay() const;

    struct qt_brcm *waylandBrcm() const;

    PFNEGLFLUSHBRCMPROC eglFlushBRCM;
    PFNEGLCREATEGLOBALIMAGEBRCMPROC eglCreateGlobalImageBRCM;
    PFNEGLDESTROYGLOBALIMAGEBRCMPROC eglDestroyGlobalImageBRCM;

private:
    static void wlDisplayHandleGlobal(void *data, struct wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

    struct wl_display *m_waylandDisplay;
    struct qt_brcm *m_waylandBrcm;

    EGLDisplay m_eglDisplay;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDBRCMEGLINTEGRATION_H
