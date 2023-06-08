// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
 * \qmlproperty list QtWayland.Compositor::WaylandCompositor::extensions
 *
 * A list of extensions that the compositor advertises to its clients. For
 * any Wayland extension the compositor should support, instantiate its component,
 * and add it to the list of extensions.
 *
 * For instance, the following code would allow the clients to request \c wl_shell
 * surfaces in the compositor using the \c wl_shell interface.
 *
 * \qml
 * import QtWayland.Compositor
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
    static_cast<QQuickWindow *>(output->window())->scheduleRenderJob(state, QQuickWindow::AfterRenderingStage);
#else
    emit grabber->failed(QWaylandSurfaceGrabber::UnknownBufferType);
#endif
}

QT_END_NAMESPACE

#include "moc_qwaylandquickcompositor.cpp"
