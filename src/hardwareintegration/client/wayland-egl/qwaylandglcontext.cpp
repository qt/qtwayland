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
#include "qwaylandeglwindow.h"

#include <QDebug>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/private/qopengltexturecache_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLShaderProgram>

QT_BEGIN_NAMESPACE

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_config(q_configFromGLFormat(m_eglDisplay, format, true))
    , m_format(q_glFormatFromConfig(m_eglDisplay, m_config))
    , m_blitProgram(0)
    , m_textureCache(0)
    , mUseNativeDefaultFbo(false)
{
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
}

QWaylandGLContext::~QWaylandGLContext()
{
    delete m_blitProgram;
    delete m_textureCache;
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);

    window->setCanResize(false);

    EGLSurface eglSurface = window->eglSurface();
    if (!eglSurface) {
        window->updateSurface(true);
        eglSurface = window->eglSurface();
    }
    if (!eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
        qWarning("QWaylandGLContext::makeCurrent: eglError: %x, this: %p \n", eglGetError(), this);
        return false;
    }

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
        makeCurrent(surface);
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

            if (!m_blitProgram->link()) {
                qDebug() << "Shader Program link failed.";
                qDebug() << m_blitProgram->log();
            }
        }

        if (!m_textureCache) {
            m_textureCache = new QOpenGLTextureCache(this->context());
        }

        QRect windowRect = window->window()->frameGeometry();
        glViewport(0, 0, windowRect.width(), windowRect.height());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
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

        m_blitProgram->bind();

        m_blitProgram->setUniformValue("texture", 0);

        m_blitProgram->enableAttributeArray("position");
        m_blitProgram->enableAttributeArray("texCoords");
        m_blitProgram->setAttributeArray("texCoords", textureVertices, 2);

        glActiveTexture(GL_TEXTURE0);

        //Draw Decoration
        m_blitProgram->setAttributeArray("position", inverseSquareVertices, 2);
        QImage decorationImage = window->decoration()->contentImage();
        m_textureCache->bindTexture(context(), decorationImage);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (!context()->functions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextureRepeat)) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Draw Content
        m_blitProgram->setAttributeArray("position", squareVertices, 2);
        glBindTexture(GL_TEXTURE_2D, window->contentTexture());
        QRect r = window->contentsRect();
        glViewport(r.x(), r.y(), r.width(), r.height());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Cleanup
        m_blitProgram->disableAttributeArray("position");
        m_blitProgram->disableAttributeArray("texCoords");
        m_blitProgram->release();
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
