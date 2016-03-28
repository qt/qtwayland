/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/QQmlEngine>
#include <QQuickWindow>
#include <QtGui/private/qopengltextureblitter_p.h>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <QRunnable>

#include "qwaylandclient.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquicksurface.h"
#include "qwaylandquickoutput.h"
#include "qwaylandquickitem.h"
#include "qwaylandoutput.h"
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include "qwaylandsurfacegrabber.h"

QT_BEGIN_NAMESPACE

class QWaylandQuickCompositorPrivate : public QWaylandCompositorPrivate
{
public:
    QWaylandQuickCompositorPrivate(QWaylandCompositor *compositor)
        : QWaylandCompositorPrivate(compositor)
    {
    }
protected:
    QWaylandSurface *createDefaultSurface() Q_DECL_OVERRIDE
    {
        return new QWaylandQuickSurface();
    }
};

QWaylandQuickCompositor::QWaylandQuickCompositor(QObject *parent)
    : QWaylandCompositor(*new QWaylandQuickCompositorPrivate(this), parent)
{
}

/*!
 * \qmlproperty list QtWaylandCompositor::WaylandCompositor::extensions
 *
 * A list of extensions that the compositor advertises to its clients. For
 * any Wayland extension the compositor should support, instantiate its component,
 * and add it to the list of extensions.
 *
 * For instance, the following code would allow the clients to request wl shell surfaces
 * in the compositor using the wl_shell interface.
 *
 * \code
 * import QtWayland.Compositor 1.0
 *
 * WaylandCompositor {
 *     extensions: [ WlShell {
 *         // ...
 *     } ]
 * }
 * \endcode
 */

void QWaylandQuickCompositor::create()
{
    QWaylandCompositor::create();
}


void QWaylandQuickCompositor::classBegin()
{
}

void QWaylandQuickCompositor::componentComplete()
{
    create();
}

/*!
 * Grab the surface content from the given \a buffer.
 * Reimplemented from QWaylandCompositor::grabSurface.
 */
void QWaylandQuickCompositor::grabSurface(QWaylandSurfaceGrabber *grabber, const QWaylandBufferRef &buffer)
{
    if (buffer.isShm()) {
        QWaylandCompositor::grabSurface(grabber, buffer);
        return;
    }

    QWaylandQuickOutput *output = static_cast<QWaylandQuickOutput *>(defaultOutput());
    if (!output) {
        emit grabber->failed(QWaylandSurfaceGrabber::RendererNotReady);
        return;
    }

    // We cannot grab the surface now, we need to have a current opengl context, so we
    // need to be in the render thread
    class GrabState : public QRunnable
    {
    public:
        QWaylandSurfaceGrabber *grabber;
        QWaylandBufferRef buffer;

        void run() Q_DECL_OVERRIDE
        {
            QOpenGLFramebufferObject fbo(buffer.size());
            fbo.bind();
            QOpenGLTextureBlitter blitter;
            blitter.create();
            blitter.bind();

            glViewport(0, 0, buffer.size().width(), buffer.size().height());

            QOpenGLTextureBlitter::Origin surfaceOrigin =
                buffer.origin() == QWaylandSurface::OriginTopLeft
                ? QOpenGLTextureBlitter::OriginTopLeft
                : QOpenGLTextureBlitter::OriginBottomLeft;

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            buffer.bindToTexture();
            blitter.blit(texture, QMatrix4x4(), surfaceOrigin);

            blitter.release();
            glDeleteTextures(1, &texture);

            emit grabber->success(fbo.toImage());
        }
    };

    GrabState *state = new GrabState;
    state->grabber = grabber;
    state->buffer = buffer;
    static_cast<QQuickWindow *>(output->window())->scheduleRenderJob(state, QQuickWindow::NoStage);
}

QT_END_NAMESPACE
