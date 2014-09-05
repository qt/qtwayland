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

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddecoration_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include "qwaylandeglwindow.h"

#include <QDebug>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/private/qopengltexturecache_p.h>
#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLShaderProgram>

QT_BEGIN_NAMESPACE

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_blitProgram(0)
    , mUseNativeDefaultFbo(false)
{
    QSurfaceFormat fmt = format;
    if (static_cast<QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration())->display()->supportsWindowDecoration())
        fmt.setAlphaBufferSize(8);
    m_config = q_configFromGLFormat(m_eglDisplay, fmt);
    m_format = q_glFormatFromConfig(m_eglDisplay, m_config);
    m_shareEGLContext = share ? static_cast<QWaylandGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;

    switch (m_format.renderableType()) {
    case QSurfaceFormat::OpenVG:
        eglBindAPI(EGL_OPENVG_API);
        break;
#ifdef EGL_VERSION_1_4
#  if !defined(QT_OPENGL_ES_2)
    case QSurfaceFormat::DefaultRenderableType:
#  endif
    case QSurfaceFormat::OpenGL:
        eglBindAPI(EGL_OPENGL_API);
        break;
#endif
    case QSurfaceFormat::OpenGLES:
    default:
        eglBindAPI(EGL_OPENGL_ES_API);
        break;
    }

    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(format.majorVersion() == 1 ? 1 : 2);
    eglContextAttrs.append(EGL_NONE);

    m_context = eglCreateContext(m_eglDisplay, m_config, m_shareEGLContext, eglContextAttrs.constData());

    if (m_context == EGL_NO_CONTEXT) {
        m_context = eglCreateContext(m_eglDisplay, m_config, EGL_NO_CONTEXT, eglContextAttrs.constData());
        m_shareEGLContext = EGL_NO_CONTEXT;
    }

    EGLint error = eglGetError();
    if (error != EGL_SUCCESS)
        qWarning("QWaylandGLContext: failed to create EGLContext, error=%x", error);
}

QWaylandGLContext::~QWaylandGLContext()
{
    delete m_blitProgram;
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);
    EGLSurface eglSurface = window->eglSurface();

    if (eglSurface != EGL_NO_SURFACE && eglGetCurrentContext() == m_context && eglGetCurrentSurface(EGL_DRAW) == eglSurface)
        return true;

    window->setCanResize(false);

    if (eglSurface == EGL_NO_SURFACE) {
        window->updateSurface(true);
        eglSurface = window->eglSurface();
    }

    if (!eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
        qWarning("QWaylandGLContext::makeCurrent: eglError: %x, this: %p \n", eglGetError(), this);
        window->setCanResize(true);
        return false;
    }

    window->bindContentFBO();

    return true;
}

void QWaylandGLContext::doneCurrent()
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

#define STATE_GUARD_VERTEX_ATTRIB_COUNT 2

class StateGuard
{
public:
    StateGuard() {
        QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

        glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &m_program);
        glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *) &m_activeTextureUnit);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &m_texture);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &m_fbo);
        glGetIntegerv(GL_VIEWPORT, m_viewport);
        glGetIntegerv(GL_DEPTH_WRITEMASK, &m_depthWriteMask);
        glGetIntegerv(GL_COLOR_WRITEMASK, m_colorWriteMask);
        m_blend = glIsEnabled(GL_BLEND);
        m_depth = glIsEnabled(GL_DEPTH_TEST);
        m_cull = glIsEnabled(GL_CULL_FACE);
        m_scissor = glIsEnabled(GL_SCISSOR_TEST);
        for (int i = 0; i < STATE_GUARD_VERTEX_ATTRIB_COUNT; ++i) {
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, (GLint *) &m_vertexAttribs[i].enabled);
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint *) &m_vertexAttribs[i].arrayBuffer);
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &m_vertexAttribs[i].size);
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &m_vertexAttribs[i].stride);
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, (GLint *) &m_vertexAttribs[i].type);
            glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, (GLint *) &m_vertexAttribs[i].normalized);
            glFuncs.glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &m_vertexAttribs[i].pointer);
        }
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint *) &m_minFilter);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint *) &m_magFilter);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint *) &m_wrapS);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint *) &m_wrapT);
    }

    ~StateGuard() {
        QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

        glFuncs.glUseProgram(m_program);
        glActiveTexture(m_activeTextureUnit);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glFuncs.glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
        glDepthMask(m_depthWriteMask);
        glColorMask(m_colorWriteMask[0], m_colorWriteMask[1], m_colorWriteMask[2], m_colorWriteMask[3]);
        if (m_blend)
            glEnable(GL_BLEND);
        if (m_depth)
            glEnable(GL_DEPTH_TEST);
        if (m_cull)
            glEnable(GL_CULL_FACE);
        if (m_scissor)
            glEnable(GL_SCISSOR_TEST);
        for (int i = 0; i < STATE_GUARD_VERTEX_ATTRIB_COUNT; ++i) {
            if (m_vertexAttribs[i].enabled)
                glFuncs.glEnableVertexAttribArray(i);
            GLuint prevBuf;
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *) &prevBuf);
            glFuncs.glBindBuffer(GL_ARRAY_BUFFER, m_vertexAttribs[i].arrayBuffer);
            glFuncs.glVertexAttribPointer(i, m_vertexAttribs[i].size, m_vertexAttribs[i].type,
                                          m_vertexAttribs[i].normalized, m_vertexAttribs[i].stride,
                                          m_vertexAttribs[i].pointer);
            glFuncs.glBindBuffer(GL_ARRAY_BUFFER, prevBuf);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
    }

private:
    GLuint m_program;
    GLenum m_activeTextureUnit;
    GLuint m_texture;
    GLuint m_fbo;
    GLint m_depthWriteMask;
    GLint m_colorWriteMask[4];
    GLboolean m_blend;
    GLboolean m_depth;
    GLboolean m_cull;
    GLboolean m_scissor;
    GLint m_viewport[4];
    struct VertexAttrib {
        bool enabled;
        GLuint arrayBuffer;
        GLint size;
        GLint stride;
        GLenum type;
        bool normalized;
        void *pointer;
    } m_vertexAttribs[STATE_GUARD_VERTEX_ATTRIB_COUNT];
    GLenum m_minFilter;
    GLenum m_magFilter;
    GLenum m_wrapS;
    GLenum m_wrapT;
};

void QWaylandGLContext::swapBuffers(QPlatformSurface *surface)
{
    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);

    EGLSurface eglSurface = window->eglSurface();

    if (window->decoration()) {
        makeCurrent(surface);

        // Must save & restore all state. Applications are usually not prepared
        // for random context state changes in a swapBuffers() call.
        StateGuard stateGuard;

        if (!m_blitProgram) {
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
        }

        QOpenGLTextureCache *cache = QOpenGLTextureCache::cacheForContext(context());

        QRect windowRect = window->window()->frameGeometry();
        glViewport(0, 0, windowRect.width(), windowRect.height());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        mUseNativeDefaultFbo = true;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mUseNativeDefaultFbo = false;

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

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_blitProgram->bind();

        m_blitProgram->enableAttributeArray(0);
        m_blitProgram->enableAttributeArray(1);
        m_blitProgram->setAttributeArray(1, textureVertices, 2);

        glActiveTexture(GL_TEXTURE0);

        //Draw Decoration
        m_blitProgram->setAttributeArray(0, inverseSquareVertices, 2);
        QImage decorationImage = window->decoration()->contentImage();
        cache->bindTexture(context(), decorationImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (context()->functions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextureRepeat)) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Draw Content
        m_blitProgram->setAttributeArray(0, squareVertices, 2);
        glBindTexture(GL_TEXTURE_2D, window->contentTexture());
        QRect r = window->contentsRect();
        glViewport(r.x(), r.y(), r.width(), r.height());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Cleanup
        m_blitProgram->disableAttributeArray(0);
        m_blitProgram->disableAttributeArray(1);
    }

    eglSwapBuffers(m_eglDisplay, eglSurface);

    window->setCanResize(true);
}

GLuint QWaylandGLContext::defaultFramebufferObject(QPlatformSurface *surface) const
{
    if (mUseNativeDefaultFbo)
        return 0;

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

void (*QWaylandGLContext::getProcAddress(const QByteArray &procName)) ()
{
    return eglGetProcAddress(procName.constData());
}

EGLConfig QWaylandGLContext::eglConfig() const
{
    return m_config;
}

QT_END_NAMESPACE
