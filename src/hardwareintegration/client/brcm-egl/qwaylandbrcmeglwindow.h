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

#ifndef QWAYLANDBRCMEGLWINDOW_H
#define QWAYLANDBRCMEGLWINDOW_H

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include "qwaylandbrcmeglintegration.h"

#include <QMutex>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBrcmBuffer;

class QWaylandBrcmEglWindow : public QWaylandWindow
{
    Q_OBJECT
public:
    QWaylandBrcmEglWindow(QWindow *window, QWaylandDisplay *display);
    ~QWaylandBrcmEglWindow();
    WindowType windowType() const override;
    void setGeometry(const QRect &rect) override;

    QSurfaceFormat format() const override;

    bool makeCurrent(EGLContext context);
    void swapBuffers();

private:
    void createEglSurfaces();
    void destroyEglSurfaces();

    QWaylandBrcmEglIntegration *m_eglIntegration = nullptr;
    struct wl_egl_window *m_waylandEglWindow = nullptr;

    const QWaylandWindow *m_parentWindow = nullptr;

    EGLint m_globalImages[3*5];
    EGLSurface m_eglSurfaces[3];

    QWaylandBrcmBuffer *m_buffers[3];
    QSurfaceFormat m_format;

    struct wl_event_queue *m_eventQueue = nullptr;

    int m_current = 0;
    int m_count = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDBRCMEGLWINDOW_H
