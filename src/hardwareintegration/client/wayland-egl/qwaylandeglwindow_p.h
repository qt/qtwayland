// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef QWAYLANDEGLWINDOW_H
#define QWAYLANDEGLWINDOW_H

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include "qwaylandeglinclude_p.h"
#include "qwaylandeglclientbufferintegration_p.h"

QT_BEGIN_NAMESPACE

class QOpenGLFramebufferObject;

namespace QtWaylandClient {

class QWaylandGLContext;

class Q_WAYLANDCLIENT_EXPORT QWaylandEglWindow : public QWaylandWindow
{
    Q_OBJECT
public:
    QWaylandEglWindow(QWindow *window, QWaylandDisplay *display);
    ~QWaylandEglWindow();
    WindowType windowType() const override;
    void ensureSize() override;

    void updateSurface(bool create);
    void setGeometry(const QRect &rect) override;
    QRect contentsRect() const;

    EGLSurface eglSurface() const;
    GLuint contentFBO() const;
    GLuint contentTexture() const;
    bool needToUpdateContentFBO() const { return decoration() && (m_resize || !m_contentFBO); }

    QSurfaceFormat format() const override;

    void bindContentFBO();

    void invalidateSurface() override;

private:
    QWaylandEglClientBufferIntegration *m_clientBufferIntegration = nullptr;
    struct wl_egl_window *m_waylandEglWindow = nullptr;

    EGLSurface m_eglSurface = EGL_NO_SURFACE;
    mutable bool m_resize = false;
    mutable QOpenGLFramebufferObject *m_contentFBO = nullptr;

    QSurfaceFormat m_format;
    QSize m_requestedSize;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDEGLWINDOW_H
