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

#include "qwaylandglcontext.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandsubsurface_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include "qwaylandeglwindow.h"

#include <QDebug>
#include <QtEglSupport/private/qeglconvenience_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/private/qopengltexturecache_p.h>
#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLBuffer>

#include <QtCore/qmutex.h>

#include <dlfcn.h>

// Constants from EGL_KHR_create_context
#ifndef EGL_CONTEXT_MINOR_VERSION_KHR
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30FB
#endif
#ifndef EGL_CONTEXT_FLAGS_KHR
#define EGL_CONTEXT_FLAGS_KHR 0x30FC
#endif
#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30FD
#endif
#ifndef EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR 0x00000001
#endif
#ifndef EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002
#endif
#ifndef EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#endif
#ifndef EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002
#endif

// Constants for OpenGL which are not available in the ES headers.
#ifndef GL_CONTEXT_FLAGS
#define GL_CONTEXT_FLAGS 0x821E
#endif
#ifndef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#endif
#ifndef GL_CONTEXT_FLAG_DEBUG_BIT
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#endif
#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif
#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif
#ifndef GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class DecorationsBlitter : public QOpenGLFunctions
{
public:
    DecorationsBlitter(QWaylandGLContext *context)
        : m_context(context)
    {
        initializeOpenGLFunctions();
        m_blitProgram = new QOpenGLShaderProgram();
        m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, "attribute vec4 position;\n\
                                                                    attribute vec4 texCoords;\n\
                                                                    varying vec2 outTexCoords;\n\
                                                                    void main()\n\
                                                                    {\n\
                                                                        gl_Position = position;\n\
                                                                        outTexCoords = texCoords.xy;\n\
                                                                    }");
        m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, "varying highp vec2 outTexCoords;\n\
                                                                        uniform sampler2D texture;\n\
                                                                        void main()\n\
                                                                        {\n\
                                                                            gl_FragColor = texture2D(texture, outTexCoords);\n\
                                                                        }");

        m_blitProgram->bindAttributeLocation("position", 0);
        m_blitProgram->bindAttributeLocation("texCoords", 1);

        if (!m_blitProgram->link()) {
            qDebug() << "Shader Program link failed.";
            qDebug() << m_blitProgram->log();
        }

        m_blitProgram->bind();
        m_blitProgram->enableAttributeArray(0);
        m_blitProgram->enableAttributeArray(1);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        m_buffer.create();
        m_buffer.bind();

        static const GLfloat squareVertices[] = {
            -1.f, -1.f,
            1.0f, -1.f,
            -1.f,  1.0f,
            1.0f,  1.0f
        };
        static const GLfloat inverseSquareVertices[] = {
            -1.f, 1.f,
            1.f, 1.f,
            -1.f, -1.f,
            1.f, -1.f
        };
        static const GLfloat textureVertices[] = {
            0.0f,  0.0f,
            1.0f,  0.0f,
            0.0f,  1.0f,
            1.0f,  1.0f,
        };

        m_squareVerticesOffset = 0;
        m_inverseSquareVerticesOffset = sizeof(squareVertices);
        m_textureVerticesOffset = sizeof(squareVertices) + sizeof(textureVertices);

        m_buffer.allocate(sizeof(squareVertices) + sizeof(inverseSquareVertices) + sizeof(textureVertices));
        m_buffer.write(m_squareVerticesOffset, squareVertices, sizeof(squareVertices));
        m_buffer.write(m_inverseSquareVerticesOffset, inverseSquareVertices, sizeof(inverseSquareVertices));
        m_buffer.write(m_textureVerticesOffset, textureVertices, sizeof(textureVertices));

        m_blitProgram->setAttributeBuffer(1, GL_FLOAT, m_textureVerticesOffset, 2);

        m_textureWrap = m_context->context()->functions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextureRepeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    }
    ~DecorationsBlitter()
    {
        delete m_blitProgram;
    }
    void blit(QWaylandEglWindow *window)
    {
        Q_ASSERT(window->wlSurface());
        QOpenGLTextureCache *cache = QOpenGLTextureCache::cacheForContext(m_context->context());

        QSize surfaceSize = window->surfaceSize();
        int scale = window->scale() ;
        glViewport(0, 0, surfaceSize.width() * scale, surfaceSize.height() * scale);

        //Draw Decoration
        m_blitProgram->setAttributeBuffer(0, GL_FLOAT, m_inverseSquareVerticesOffset, 2);
        QImage decorationImage = window->decoration()->contentImage();
        cache->bindTexture(m_context->context(), decorationImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_textureWrap);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Draw Content
        m_blitProgram->setAttributeBuffer(0, GL_FLOAT, m_squareVerticesOffset, 2);
        glBindTexture(GL_TEXTURE_2D, window->contentTexture());
        QRect r = window->contentsRect();
        glViewport(r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    QOpenGLShaderProgram *m_blitProgram = nullptr;
    QWaylandGLContext *m_context = nullptr;
    QOpenGLBuffer m_buffer;
    int m_squareVerticesOffset;
    int m_inverseSquareVerticesOffset;
    int m_textureVerticesOffset;
    int m_textureWrap;
};



QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_display(display)
{
    QSurfaceFormat fmt = format;
    if (static_cast<QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration())->display()->supportsWindowDecoration())
        fmt.setAlphaBufferSize(8);
    m_config = q_configFromGLFormat(m_eglDisplay, fmt);
    m_format = q_glFormatFromConfig(m_eglDisplay, m_config, fmt);
    m_shareEGLContext = share ? static_cast<QWaylandGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;

    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(format.majorVersion());
    const bool hasKHRCreateContext = q_hasEglExtension(m_eglDisplay, "EGL_KHR_create_context");
    if (hasKHRCreateContext) {
        eglContextAttrs.append(EGL_CONTEXT_MINOR_VERSION_KHR);
        eglContextAttrs.append(format.minorVersion());
        int flags = 0;
        // The debug bit is supported both for OpenGL and OpenGL ES.
        if (format.testOption(QSurfaceFormat::DebugContext))
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        // The fwdcompat bit is only for OpenGL 3.0+.
        if (m_format.renderableType() == QSurfaceFormat::OpenGL
            && format.majorVersion() >= 3
            && !format.testOption(QSurfaceFormat::DeprecatedFunctions))
            flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;
        if (flags) {
            eglContextAttrs.append(EGL_CONTEXT_FLAGS_KHR);
            eglContextAttrs.append(flags);
        }
        // Profiles are OpenGL only and mandatory in 3.2+. The value is silently ignored for < 3.2.
        if (m_format.renderableType() == QSurfaceFormat::OpenGL) {
            switch (format.profile()) {
            case QSurfaceFormat::NoProfile:
                break;
            case QSurfaceFormat::CoreProfile:
                eglContextAttrs.append(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR);
                eglContextAttrs.append(EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR);
                break;
            case QSurfaceFormat::CompatibilityProfile:
                eglContextAttrs.append(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR);
                eglContextAttrs.append(EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR);
                break;
            }
        }
    }
    eglContextAttrs.append(EGL_NONE);

    switch (m_format.renderableType()) {
    case QSurfaceFormat::OpenVG:
        m_api = EGL_OPENVG_API;
        break;
#ifdef EGL_VERSION_1_4
#  if !defined(QT_OPENGL_ES_2)
    case QSurfaceFormat::DefaultRenderableType:
#  endif
    case QSurfaceFormat::OpenGL:
        m_api = EGL_OPENGL_API;
        break;
#endif
    case QSurfaceFormat::OpenGLES:
    default:
        m_api = EGL_OPENGL_ES_API;
        break;
    }
    eglBindAPI(m_api);

    m_context = eglCreateContext(m_eglDisplay, m_config, m_shareEGLContext, eglContextAttrs.constData());

    if (m_context == EGL_NO_CONTEXT) {
        m_context = eglCreateContext(m_eglDisplay, m_config, EGL_NO_CONTEXT, eglContextAttrs.constData());
        m_shareEGLContext = EGL_NO_CONTEXT;
    }

    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        qWarning("QWaylandGLContext: failed to create EGLContext, error=%x", error);
        return;
    }

    // Create an EGL context for the decorations blitter. By using a dedicated context we don't need to make sure to not
    // change the context state and we also use OpenGL ES 2 API independently to what the app is using to draw.
    QVector<EGLint> eglDecorationsContextAttrs = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_decorationsContext = eglCreateContext(m_eglDisplay, m_config, m_context, eglDecorationsContextAttrs.constData());
    if (m_decorationsContext == EGL_NO_CONTEXT)
        qWarning("QWaylandGLContext: Failed to create the decorations EGLContext. Decorations will not be drawn.");

    EGLint a = EGL_MIN_SWAP_INTERVAL;
    EGLint b = EGL_MAX_SWAP_INTERVAL;
    if (!eglGetConfigAttrib(m_eglDisplay, m_config, a, &a) ||
        !eglGetConfigAttrib(m_eglDisplay, m_config, b, &b) ||
        a > 0) {
       m_supportNonBlockingSwap = false;
    }
    {
        bool ok;
        int supportNonBlockingSwap = qEnvironmentVariableIntValue("QT_WAYLAND_FORCE_NONBLOCKING_SWAP_SUPPORT", &ok);
        if (ok)
            m_supportNonBlockingSwap = supportNonBlockingSwap != 0;
    }
    if (!m_supportNonBlockingSwap) {
        qWarning(lcQpaWayland) << "Non-blocking swap buffers not supported."
                               << "Subsurface rendering can be affected."
                               << "It may also cause the event loop to freeze in some situations";
    }

    updateGLFormat();
}

void QWaylandGLContext::updateGLFormat()
{
    // Have to save & restore to prevent QOpenGLContext::currentContext() from becoming
    // inconsistent after QOpenGLContext::create().
    EGLDisplay prevDisplay = eglGetCurrentDisplay();
    if (prevDisplay == EGL_NO_DISPLAY) // when no context is current
        prevDisplay = m_eglDisplay;
    EGLContext prevContext = eglGetCurrentContext();
    EGLSurface prevSurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface prevSurfaceRead = eglGetCurrentSurface(EGL_READ);

    wl_surface *wlSurface = m_display->createSurface(nullptr);
    wl_egl_window *eglWindow = wl_egl_window_create(wlSurface, 1, 1);
#if QT_CONFIG(egl_extension_platform_wayland)
    EGLSurface eglSurface = eglCreatePlatformWindowSurface(m_eglDisplay, m_config, eglWindow, nullptr);
#else
    EGLSurface eglSurface = eglCreateWindowSurface(m_eglDisplay, m_config, eglWindow, nullptr);
#endif

    if (eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
        if (m_format.renderableType() == QSurfaceFormat::OpenGL
            || m_format.renderableType() == QSurfaceFormat::OpenGLES) {
            const GLubyte *s = glGetString(GL_VERSION);
            if (s) {
                QByteArray version = QByteArray(reinterpret_cast<const char *>(s));
                int major, minor;
                if (QPlatformOpenGLContext::parseOpenGLVersion(version, major, minor)) {
                    m_format.setMajorVersion(major);
                    m_format.setMinorVersion(minor);
                }
            }
            m_format.setProfile(QSurfaceFormat::NoProfile);
            m_format.setOptions(QSurfaceFormat::FormatOptions());
            if (m_format.renderableType() == QSurfaceFormat::OpenGL) {
                // Check profile and options.
                if (m_format.majorVersion() < 3) {
                    m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
                } else {
                    GLint value = 0;
                    glGetIntegerv(GL_CONTEXT_FLAGS, &value);
                    if (!(value & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT))
                        m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
                    if (value & GL_CONTEXT_FLAG_DEBUG_BIT)
                        m_format.setOption(QSurfaceFormat::DebugContext);
                    if (m_format.version() >= qMakePair(3, 2)) {
                        value = 0;
                        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &value);
                        if (value & GL_CONTEXT_CORE_PROFILE_BIT)
                            m_format.setProfile(QSurfaceFormat::CoreProfile);
                        else if (value & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
                            m_format.setProfile(QSurfaceFormat::CompatibilityProfile);
                    }
                }
            }
        }
        eglMakeCurrent(prevDisplay, prevSurfaceDraw, prevSurfaceRead, prevContext);
    }
    eglDestroySurface(m_eglDisplay, eglSurface);
    wl_egl_window_destroy(eglWindow);
    wl_surface_destroy(wlSurface);
}

QWaylandGLContext::~QWaylandGLContext()
{
    delete m_blitter;
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
    // in QWaylandGLContext() we called eglBindAPI with the correct value. However,
    // eglBindAPI's documentation says:
    // "eglBindAPI defines the current rendering API for EGL in the thread it is called from"
    // Since makeCurrent() can be called from a different thread than the one we created the
    // context in make sure to call eglBindAPI in the correct thread.
    if (eglQueryAPI() != m_api) {
        eglBindAPI(m_api);
    }

    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);
    EGLSurface eglSurface = window->eglSurface();

    if (!window->needToUpdateContentFBO() && (eglSurface != EGL_NO_SURFACE)) {
        if (!eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
            qWarning("QWaylandGLContext::makeCurrent: eglError: %x, this: %p \n", eglGetError(), this);
            return false;
        }
        return true;
    }

    if (window->isExposed())
        window->setCanResize(false);
    if (m_decorationsContext != EGL_NO_CONTEXT && !window->decoration())
        window->createDecoration();

    if (eglSurface == EGL_NO_SURFACE) {
        window->updateSurface(true);
        eglSurface = window->eglSurface();
    }

    if (!eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
        qWarning("QWaylandGLContext::makeCurrent: eglError: %x, this: %p \n", eglGetError(), this);
        window->setCanResize(true);
        return false;
    }

    //### setCurrentContext will be called in QOpenGLContext::makeCurrent after this function
    // returns, but that's too late, as we need a current context in order to bind the content FBO.
    QOpenGLContextPrivate::setCurrentContext(context());
    window->bindContentFBO();

    return true;
}

void QWaylandGLContext::doneCurrent()
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void QWaylandGLContext::swapBuffers(QPlatformSurface *surface)
{
    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);

    EGLSurface eglSurface = window->eglSurface();

    if (window->decoration()) {
        if (m_api != EGL_OPENGL_ES_API)
            eglBindAPI(EGL_OPENGL_ES_API);

        // save the current EGL content and surface to set it again after the blitter is done
        EGLDisplay currentDisplay = eglGetCurrentDisplay();
        EGLContext currentContext = eglGetCurrentContext();
        EGLSurface currentSurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
        EGLSurface currentSurfaceRead = eglGetCurrentSurface(EGL_READ);
        eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_decorationsContext);

        if (!m_blitter)
            m_blitter = new DecorationsBlitter(this);
        m_blitter->blit(window);

        if (m_api != EGL_OPENGL_ES_API)
            eglBindAPI(m_api);
        eglMakeCurrent(currentDisplay, currentSurfaceDraw, currentSurfaceRead, currentContext);
    }

    int swapInterval = m_supportNonBlockingSwap ? 0 : m_format.swapInterval();
    eglSwapInterval(m_eglDisplay, swapInterval);
    if (swapInterval == 0 && m_format.swapInterval() > 0) {
        // Emulating a blocking swap
        glFlush(); // Flush before waiting so we can swap more quickly when the frame event arrives
        window->waitForFrameSync(100);
    }
    window->handleUpdate();
    eglSwapBuffers(m_eglDisplay, eglSurface);

    window->setCanResize(true);
}

GLuint QWaylandGLContext::defaultFramebufferObject(QPlatformSurface *surface) const
{
    return static_cast<QWaylandEglWindow *>(surface)->contentFBO();
}

bool QWaylandGLContext::isSharing() const
{
    return m_shareEGLContext != EGL_NO_CONTEXT;
}

bool QWaylandGLContext::isValid() const
{
    return m_context != EGL_NO_CONTEXT;
}

QFunctionPointer QWaylandGLContext::getProcAddress(const char *procName)
{
    QFunctionPointer proc = (QFunctionPointer) eglGetProcAddress(procName);
    if (!proc)
        proc = (QFunctionPointer) dlsym(RTLD_DEFAULT, procName);
    return proc;
}

EGLConfig QWaylandGLContext::eglConfig() const
{
    return m_config;
}

}

QT_END_NAMESPACE
