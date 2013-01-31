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

#include "qwaylanddisplay.h"
#include "qwaylandwindow.h"
#include "qwaylandeglwindow.h"
#include "qwaylanddecoration.h"

#include <QDebug>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtGui/private/qopenglcontext_p.h>

#include <qpa/qplatformopenglcontext.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLShaderProgram>

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, const QSurfaceFormat &format, QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
    , m_eglDisplay(eglDisplay)
    , m_config(q_configFromGLFormat(m_eglDisplay, format, true))
    , m_format(q_glFormatFromConfig(m_eglDisplay, m_config))
    , m_blitProgram(0)
{
    m_shareEGLContext = share ? static_cast<QWaylandGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;

    eglBindAPI(EGL_OPENGL_ES_API);

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
    eglDestroyContext(m_eglDisplay, m_context);
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
    QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);
    EGLSurface eglSurface = window->eglSurface();
    if (!eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context))
        return false;

    // FIXME: remove this as soon as https://codereview.qt-project.org/#change,38879 is merged
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
        makeCurrent(surface);

        if (!m_blitProgram) {
            m_blitProgram = new QOpenGLShaderProgram();
            m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, "attribute vec4 position;\n\
                                                                        attribute vec4 texCoords;\n\
                                                                        varying vec2 outTexCoords;\n\
                                                                        void main()\n\
                                                                        {\n\
                                                                            gl_Position = position;\n\
                                                                            outTexCoords = texCoords.xy;\n\
                                                                        }");
            m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, "varying vec2 outTexCoords;\n\
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

        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (window->decoration())
            window->decoration()->paintDecoration();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, window->contentTexture());
        QRect r = window->contentsRect();
        glViewport(r.x(), r.y(), r.width(), r.height());

        static const GLfloat squareVertices[] = {
            -1.f, -1.f,
            1.0f, -1.f,
            -1.f,  1.0f,
            1.0f,  1.0f,
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
        m_blitProgram->setAttributeArray("position", squareVertices, 2);
        m_blitProgram->setAttributeArray("texCoords", textureVertices, 2);

        m_blitProgram->bind();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_blitProgram->disableAttributeArray("position");
        m_blitProgram->disableAttributeArray("texCoords");
        m_blitProgram->release();
    }

    eglSwapBuffers(m_eglDisplay, eglSurface);
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

void (*QWaylandGLContext::getProcAddress(const QByteArray &procName)) ()
{
    return eglGetProcAddress(procName.constData());
}

EGLConfig QWaylandGLContext::eglConfig() const
{
    return m_config;
}

