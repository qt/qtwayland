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

#include "qwaylandreadbackcglcontext.h"

#include "qwaylandshmbackingstore.h"
#include "qwaylandreadbackcglwindow.h"

#include <QtGui/QOpenGLContext>
#include <QtCore/QDebug>

#include <OpenGL/OpenGL.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

#include <QtPlatformSupport/private/cglconvenience_p.h>

QWaylandReadbackCGLContext::QWaylandReadbackCGLContext(QPlatformOpenGLContext *share)
    : QPlatformOpenGLContext()
{
    Q_UNUSED(share);
    m_glContext = qcgl_createGlContext();
}

QSurfaceFormat QWaylandReadbackCGLContext::format() const
{
    return qcgl_surfaceFormat();
}

bool QWaylandReadbackCGLContext::makeCurrent(QPlatformSurface *surface)
{
    QWaylandReadbackCGLWindow *window = static_cast<QWaylandReadbackCGLWindow *>(surface);
    CGLSetPBuffer(m_glContext, window->pixelBuffer(), 0, 0, 0);
    CGLSetCurrentContext(m_glContext);
    return true;
}

void QWaylandReadbackCGLContext::doneCurrent()
{
    CGLSetCurrentContext(0);
}

void QWaylandReadbackCGLContext::swapBuffers(QPlatformSurface *surface)
{
    Q_UNUSED(surface);

    if (QOpenGLContext::currentContext()->handle() != this) {
        makeCurrent(surface);
    }
    CGLFlushDrawable(m_glContext);

    QWaylandReadbackCGLWindow *window = static_cast<QWaylandReadbackCGLWindow *>(surface);
    QSize size = window->geometry().size();

    uchar *dstBits = const_cast<uchar *>(window->buffer());
    glReadPixels(0,0, size.width(), size.height(), GL_BGRA,GL_UNSIGNED_BYTE, dstBits);

    window->damage(QRect(QPoint(0,0),size));

    // ### Should sync here but this call deadlocks with the server.
    //window->waitForFrameSync();
}

void (*QWaylandReadbackCGLContext::getProcAddress(const QByteArray &procName)) ()
{
    return qcgl_getProcAddress(procName);
}

