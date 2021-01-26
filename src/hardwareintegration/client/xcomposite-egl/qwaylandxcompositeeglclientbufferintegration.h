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

#ifndef QWAYLANDXCOMPOSITEEGLCLIENTBUFFERINTEGRATION_H
#define QWAYLANDXCOMPOSITEEGLCLIENTBUFFERINTEGRATION_H

#include <QtWaylandClient/private/qwaylandclientbufferintegration_p.h>
#include <wayland-client-core.h>

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWindow>

#include <qpa/qplatformopenglcontext.h>

#include <QWaitCondition>

#include <X11/Xlib.h>
#include <EGL/egl.h>

// avoid clashes with Qt::CursorShape
#ifdef CursorShape
#   define X_CursorShape CursorShape
#   undef CursorShape
#endif

struct qt_xcomposite;
struct qt_xcomposite_listener;
struct wl_registry;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandXCompositeEGLClientBufferIntegration : public QWaylandClientBufferIntegration
{
public:
    QWaylandXCompositeEGLClientBufferIntegration();
    ~QWaylandXCompositeEGLClientBufferIntegration() override;

    void initialize(QWaylandDisplay *dispaly) override;

    QWaylandWindow *createEglWindow(QWindow *window) override;
    QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const override;

    QWaylandDisplay *waylandDisplay() const;
    struct qt_xcomposite *waylandXComposite() const;

    Display *xDisplay() const;
    EGLDisplay eglDisplay() const;
    int screen() const;
    Window rootWindow() const;

    bool supportsThreadedOpenGL() const override { return true; }
    bool supportsWindowDecoration() const override { return false; }

private:
    QWaylandDisplay *mWaylandDisplay = nullptr;
    struct qt_xcomposite *mWaylandComposite = nullptr;

    Display *mDisplay = nullptr;
    EGLDisplay mEglDisplay = EGL_NO_DISPLAY;
    int mScreen = 0;
    Window mRootWindow = -1;

    static void wlDisplayHandleGlobal(void *data, struct ::wl_registry *registry, uint32_t id,
                                      const QString &interface, uint32_t version);

    static const struct ::qt_xcomposite_listener xcomposite_listener;
    static void rootInformation(void *data,
                 struct qt_xcomposite *xcomposite,
                 const char *display_name,
                 uint32_t root_window);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXCOMPOSITEEGLCLIENTBUFFERINTEGRATION_H
