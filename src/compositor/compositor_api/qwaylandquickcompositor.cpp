/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/QQmlEngine>
#include <QQuickWindow>
#if QT_CONFIG(opengl)
#  include <QOpenGLTextureBlitter>
#  include <QOpenGLTexture>
#  include <QOpenGLFramebufferObject>
#endif
#include <QMatrix4x4>
#include <QRunnable>

#include "qwaylandclient.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquicksurface.h"
#include "qwaylandquickoutput.h"
#include "qwaylandquickitem.h"
#include "qwaylandoutput.h"
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/QWaylandViewporter>
#include "qwaylandsurfacegrabber.h"

QT_BEGIN_NAMESPACE

class QWaylandQuickCompositorPrivate : public QWaylandCompositorPrivate
{
public:
    explicit QWaylandQuickCompositorPrivate(QWaylandCompositor *compositor)
        : QWaylandCompositorPrivate(compositor)
        , m_viewporter(new QWaylandViewporter(compositor))
    {
    }
protected:
    QWaylandSurface *createDefaultSurface() override
    {
        return new QWaylandQuickSurface();
    }
private:
    QScopedPointer<QWaylandViewporter> m_viewporter;
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
 * For instance, the following code would allow the clients to request \c wl_shell
 * surfaces in the compositor using the \c wl_shell interface.
 *
 * \qml \QtMinorVersion
 * import QtWayland.Compositor 1.\1
 *
 * WaylandCompositor {
 *     WlShell {
 *         // ...
 *     }
 * }
 * \endqml
 */

void QWaylandQuickCompositor::create()
{
    QWaylandCompositor::create();
}


void QWaylandQuickCompositor::classBegin()
{
    QWaylandCompositorPrivate::get(this)->preInit();
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
    if (buffer.isSharedMemory()) {
        QWaylandCompositor::grabSurface(grabber, buffer);
        return;
    }

#if QT_CONFIG(opengl)
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
        QWaylandSurfaceGrabber *grabber = nullptr;
        QWaylandBufferRef buffer;

        void run() override
        {
            QOpenGLFramebufferObject fbo(buffer.size());
            fbo.bind();
            QOpenGLTextureBlitter blitter;
            blitter.create();

            glViewport(0, 0, buffer.size().width(), buffer.size().height());

            QOpenGLTextureBlitter::Origin surfaceOrigin =
                buffer.origin() == QWaylandSurface::OriginTopLeft
                ? QOpenGLTextureBlitter::OriginTopLeft
                : QOpenGLTextureBlitter::OriginBottomLeft;

            auto texture = buffer.toOpenGLTexture();
            blitter.bind(texture->target());
            blitter.blit(texture->textureId(), QMatrix4x4(), surfaceOrigin);
            blitter.release();

            emit grabber->success(fbo.toImage());
        }
    };

    GrabState *state = new GrabState;
    state->grabber = grabber;
    state->buffer = buffer;
    static_cast<QQuickWindow *>(output->window())->scheduleRenderJob(state, QQuickWindow::NoStage);
#else
    emit grabber->failed(QWaylandSurfaceGrabber::UnknownBufferType);
#endif
}

QT_END_NAMESPACE
