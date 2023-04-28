// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandeglwindow_p.h"

#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include "qwaylandglcontext_p.h"

#include <QtGui/private/qeglconvenience_p.h>

#include <QDebug>
#include <QtGui/QWindow>
#include <qpa/qwindowsysteminterface.h>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandEglWindow::QWaylandEglWindow(QWindow *window, QWaylandDisplay *display)
    : QWaylandWindow(window, display)
    , m_clientBufferIntegration(static_cast<QWaylandEglClientBufferIntegration *>(mDisplay->clientBufferIntegration()))
    , m_format(window->requestedFormat())
{
    connect(display, &QWaylandDisplay::reconnected, this, [this] {
        m_clientBufferIntegration = static_cast<QWaylandEglClientBufferIntegration *>(
                mDisplay->clientBufferIntegration());
    });
}

QWaylandEglWindow::~QWaylandEglWindow()
{
    if (m_eglSurface) {
        eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
        m_eglSurface = 0;
    }

    if (m_waylandEglWindow)
        wl_egl_window_destroy(m_waylandEglWindow);

    delete m_contentFBO;
}

QWaylandWindow::WindowType QWaylandEglWindow::windowType() const
{
    return QWaylandWindow::Egl;
}

void QWaylandEglWindow::ensureSize()
{
    updateSurface(false);
}

void QWaylandEglWindow::setGeometry(const QRect &rect)
{
    QWaylandWindow::setGeometry(rect);
    // If the surface was invalidated through invalidateSurface() and
    // we're now getting a resize we don't want to create it again.
    // Just resize the wl_egl_window, the EGLSurface will be created
    // the next time makeCurrent is called.
    updateSurface(false);
}

void QWaylandEglWindow::updateSurface(bool create)
{
    QMargins margins = mWindowDecoration ? frameMargins() : QMargins{};
    QRect rect = geometry();
    QSize sizeWithMargins = (rect.size() + QSize(margins.left() + margins.right(), margins.top() + margins.bottom())) * scale();

    // wl_egl_windows must have both width and height > 0
    // mesa's egl returns NULL if we try to create a, invalid wl_egl_window, however not all EGL
    // implementations may do that, so check the size ourself. Besides, we must deal with resizing
    // a valid window to 0x0, which would make it invalid. Hence, destroy it.
    if (sizeWithMargins.isEmpty()) {
        if (m_eglSurface) {
            eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
            m_eglSurface = 0;
        }
        if (m_waylandEglWindow) {
            wl_egl_window_destroy(m_waylandEglWindow);
            m_waylandEglWindow = 0;
        }
        mOffset = QPoint();
    } else {
        QReadLocker locker(&mSurfaceLock);
        if (m_waylandEglWindow) {
            int current_width, current_height;
            static bool disableResizeCheck = qgetenv("QT_WAYLAND_DISABLE_RESIZECHECK").toInt();

            if (!disableResizeCheck) {
                wl_egl_window_get_attached_size(m_waylandEglWindow, &current_width, &current_height);
            }
            if (disableResizeCheck || (current_width != sizeWithMargins.width() || current_height != sizeWithMargins.height()) || m_requestedSize != sizeWithMargins) {
                wl_egl_window_resize(m_waylandEglWindow, sizeWithMargins.width(), sizeWithMargins.height(), mOffset.x(), mOffset.y());
                m_requestedSize = sizeWithMargins;
                mOffset = QPoint();

                m_resize = true;
            }
        } else if (create && mSurface) {
            m_waylandEglWindow = wl_egl_window_create(mSurface->object(), sizeWithMargins.width(), sizeWithMargins.height());
            m_requestedSize = sizeWithMargins;
        }

        if (!m_eglSurface && m_waylandEglWindow && create) {
            EGLNativeWindowType eglw = (EGLNativeWindowType) m_waylandEglWindow;
            QSurfaceFormat fmt = window()->requestedFormat();
            if (mDisplay->supportsWindowDecoration())
                fmt.setAlphaBufferSize(8);
            EGLConfig eglConfig = q_configFromGLFormat(m_clientBufferIntegration->eglDisplay(), fmt);
            m_format = q_glFormatFromConfig(m_clientBufferIntegration->eglDisplay(), eglConfig);
            m_eglSurface = eglCreateWindowSurface(m_clientBufferIntegration->eglDisplay(), eglConfig, eglw, 0);
            if (Q_UNLIKELY(m_eglSurface == EGL_NO_SURFACE))
                qCWarning(lcQpaWayland, "Could not create EGL surface (EGL error 0x%x)\n", eglGetError());
        }
    }
}

QRect QWaylandEglWindow::contentsRect() const
{
    QRect r = geometry();
    QMargins m = frameMargins();
    return QRect(m.left(), m.bottom(), r.width(), r.height());
}

QSurfaceFormat QWaylandEglWindow::format() const
{
    return m_format;
}

void QWaylandEglWindow::invalidateSurface()
{
    if (m_eglSurface) {
        eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
        m_eglSurface = 0;
    }
    if (m_waylandEglWindow) {
        wl_egl_window_destroy(m_waylandEglWindow);
        m_waylandEglWindow = nullptr;
    }
    delete m_contentFBO;
    m_contentFBO = nullptr;
}

EGLSurface QWaylandEglWindow::eglSurface() const
{
    return m_eglSurface;
}

GLuint QWaylandEglWindow::contentFBO() const
{
    if (!decoration())
        return 0;

    if (m_resize || !m_contentFBO) {
        QOpenGLFramebufferObject *old = m_contentFBO;
        QSize fboSize = geometry().size() * scale();
        m_contentFBO = new QOpenGLFramebufferObject(fboSize.width(), fboSize.height(), QOpenGLFramebufferObject::CombinedDepthStencil);

        delete old;
        m_resize = false;
    }

    return m_contentFBO->handle();
}

GLuint QWaylandEglWindow::contentTexture() const
{
    return m_contentFBO->texture();
}

void QWaylandEglWindow::bindContentFBO()
{
    if (decoration()) {
        contentFBO();
        m_contentFBO->bind();
    }
}

}

QT_END_NAMESPACE

#include "moc_qwaylandeglwindow_p.cpp"
