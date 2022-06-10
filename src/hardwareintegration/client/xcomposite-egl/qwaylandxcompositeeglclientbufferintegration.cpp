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

#include "qwaylandxcompositeeglclientbufferintegration.h"

#include "qwaylandxcompositeeglwindow.h"

#include <QtCore/QDebug>
#include <QtGui/private/qeglconvenience_p.h>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#include "wayland-xcomposite-client-protocol.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXCompositeEGLClientBufferIntegration::QWaylandXCompositeEGLClientBufferIntegration()
    : QWaylandClientBufferIntegration()
{
    qDebug() << "Using XComposite-EGL";
}

QWaylandXCompositeEGLClientBufferIntegration::~QWaylandXCompositeEGLClientBufferIntegration()
{
    XCloseDisplay(mDisplay);
}

void QWaylandXCompositeEGLClientBufferIntegration::initialize(QWaylandDisplay *display)
{
    mWaylandDisplay = display;
    mWaylandDisplay->addRegistryListener(&wlDisplayHandleGlobal, this);
    while (!mDisplay) {
        display->flushRequests();
        display->blockingReadEvents();
    }
}

QWaylandWindow * QWaylandXCompositeEGLClientBufferIntegration::createEglWindow(QWindow *window)
{
    return new QWaylandXCompositeEGLWindow(window,this);
}

QPlatformOpenGLContext *QWaylandXCompositeEGLClientBufferIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const
{
    EGLDisplay display = eglDisplay();
    EGLConfig config = q_configFromGLFormat(display, glFormat, true, EGL_WINDOW_BIT | EGL_PIXMAP_BIT);
    return new QWaylandXCompositeEGLContext(glFormat, share, display, config);
}

Display * QWaylandXCompositeEGLClientBufferIntegration::xDisplay() const
{
    return mDisplay;
}

EGLDisplay QWaylandXCompositeEGLClientBufferIntegration::eglDisplay() const
{
    return mEglDisplay;
}

int QWaylandXCompositeEGLClientBufferIntegration::screen() const
{
    return mScreen;
}

Window QWaylandXCompositeEGLClientBufferIntegration::rootWindow() const
{
    return mRootWindow;
}

QWaylandDisplay * QWaylandXCompositeEGLClientBufferIntegration::waylandDisplay() const
{
    return mWaylandDisplay;
}
qt_xcomposite * QWaylandXCompositeEGLClientBufferIntegration::waylandXComposite() const
{
    return mWaylandComposite;
}

const struct qt_xcomposite_listener QWaylandXCompositeEGLClientBufferIntegration::xcomposite_listener = {
    QWaylandXCompositeEGLClientBufferIntegration::rootInformation
};

void QWaylandXCompositeEGLClientBufferIntegration::wlDisplayHandleGlobal(void *data, ::wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    Q_UNUSED(version);
    if (interface == "qt_xcomposite") {
        QWaylandXCompositeEGLClientBufferIntegration *integration = static_cast<QWaylandXCompositeEGLClientBufferIntegration *>(data);
        integration->mWaylandComposite = static_cast<struct qt_xcomposite *>(wl_registry_bind(registry,id,&qt_xcomposite_interface,1));
        qt_xcomposite_add_listener(integration->mWaylandComposite,&xcomposite_listener,integration);
    }

}

void QWaylandXCompositeEGLClientBufferIntegration::rootInformation(void *data, qt_xcomposite *xcomposite, const char *display_name, uint32_t root_window)
{
    Q_UNUSED(xcomposite);
    QWaylandXCompositeEGLClientBufferIntegration *integration = static_cast<QWaylandXCompositeEGLClientBufferIntegration *>(data);

    integration->mDisplay = XOpenDisplay(display_name);
    integration->mRootWindow = (Window) root_window;
    integration->mScreen = XDefaultScreen(integration->mDisplay);
    integration->mEglDisplay = eglGetDisplay(integration->mDisplay);
    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint minor,major;
    if (!eglInitialize(integration->mEglDisplay,&major,&minor)) {
        qFatal("Failed to initialize EGL");
    }
    eglSwapInterval(integration->eglDisplay(),0);
    qDebug() << "ROOT INFORMATION" << integration->mDisplay << integration->mRootWindow << integration->mScreen;
}

}

QT_END_NAMESPACE