/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandcompositor.h"

#include "qwaylandsurface.h"
#include <QtCompositor/qwaylandinput.h>

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QtCore/QLinkedList>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QOpenGLContext>
#include <QGLWidget>
#include <QtGui/private/qopengltexturecache_p.h>
#include "textureblitter.h"
#include <QOpenGLFunctions>
#endif

#include <QDebug>

#ifdef QT_COMPOSITOR_WAYLAND_GL
class QWidgetCompositor : public QGLWidget, public QWaylandCompositor
#else
class QWidgetCompositor : public QWidget, public WaylandCompositor
#endif
{
    Q_OBJECT
public:
    QWidgetCompositor()
        : QWaylandCompositor(windowHandle(), 0, DefaultExtensions | SubSurfaceExtension)
#ifdef QT_COMPOSITOR_WAYLAND_GL
        , m_surfaceCompositorFbo(0)
        , m_textureBlitter(0)
        , m_textureCache(0)
#endif
        , m_moveSurface(0)
        , m_dragSourceSurface(0)
        , m_cursorSurface(0)
    {
        setMouseTracking(true);
        setRetainedSelectionEnabled(true);
        m_background = QImage(QLatin1String(":/background.jpg"));
        //make sure we get the window id and create the glcontext
        //so that clients can successfully initialize egl
        winId();
    }

private slots:
    void surfaceDestroyed(QObject *object) {
        QWaylandSurface *surface = static_cast<QWaylandSurface *>(object);
        m_surfaces.removeAll(surface);
        update();
    }

    void surfaceMapped() {
        QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
        QPoint pos;
        if (!m_surfaces.contains(surface)) {
            uint px = 1 + (qrand() % (width() - surface->size().width() - 2));
            uint py = 1 + (qrand() % (height() - surface->size().height() - 2));
            pos = QPoint(px, py);
            surface->setPos(pos);
            if (surface->hasShellSurface())
                m_surfaces.append(surface);
        }
        if (surface->hasShellSurface())
            defaultInputDevice()->setKeyboardFocus(surface);
        update();
    }

    void surfaceDamaged(const QRect &rect) {
        QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
        surfaceDamaged(surface, rect);
    }

protected:
    void surfaceDamaged(QWaylandSurface *surface, const QRect &rect)
    {
#ifdef QT_COMPOSITOR_WAYLAND_GL
        Q_UNUSED(surface);
        Q_UNUSED(rect);
        update();
#else
        update(rect.translated(surface->geometry().topLeft()));
#endif
    }

    void surfaceCreated(QWaylandSurface *surface) {
        connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
        connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
        connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
        update();
    }

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint composeSurface(QWaylandSurface *surface) {
        GLuint texture = 0;
        QOpenGLFunctions *functions = QOpenGLContext::currentContext()->functions();

        QSize windowSize = surface->size();
        surface->advanceBufferQueue();

        if (!m_surfaceCompositorFbo)
            functions->glGenFramebuffers(1,&m_surfaceCompositorFbo);

        functions->glBindFramebuffer(GL_FRAMEBUFFER, m_surfaceCompositorFbo);

        if (surface->type() == QWaylandSurface::Shm) {
            texture = m_textureCache->bindTexture(context()->contextHandle(), surface->image());
        } else {
            texture = surface->texture();
        }

        functions->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, texture, 0);
        paintChildren(surface,surface,windowSize);
        functions->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, 0, 0);

        functions->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return texture;
    }

    void paintChildren(QWaylandSurface *surface, QWaylandSurface *window, const QSize &windowSize) {

        if (surface->subSurfaces().size() == 0)
            return;

        QLinkedListIterator<QWaylandSurface *> i(surface->subSurfaces());
        while (i.hasNext()) {
            QWaylandSurface *subSurface = i.next();
            QPointF p = subSurface->mapTo(window,QPoint(0,0));
            QSize size = subSurface->size();
            subSurface->advanceBufferQueue();
            if (size.isValid()) {
                GLuint texture = 0;
                if (subSurface->type() == QWaylandSurface::Texture) {
                    texture = subSurface->texture();
                } else if (surface->type() == QWaylandSurface::Shm ) {
                    texture = m_textureCache->bindTexture(context()->contextHandle(), surface->image());
                }
                m_textureBlitter->drawTexture(texture,QRect(p.toPoint(),size),windowSize,0,window->isYInverted(),subSurface->isYInverted());
            }
            paintChildren(subSurface,window,windowSize);
        }
    }
#else //hmmm, this is actually untested :(
    QImage composeSurface(WaylandSurface *surface)
    {
        Q_ASSER(surface->type() == WaylandSurface::Shm);
        QImage img = surface->image();
        QPainter p(&img);
        paintChildren(surface,p,surface);

        return img;
    }

    void paintChildren(WaylandSurface *surface, QPainter *painter, WaylandSurface *window) {
        if (surface->subSurfaces().size() == 0)
            return;

        QLinkedListIterator<WaylandSurface *> i(surface->subSurfaces());
        while (i.hasNext()) {
            WaylandSurface *subSurface = i.next();
            QPoint p = subSurface->mapTo(window,QPoint(0,0));
            QRect geo = subSurface->geometry();
            geo.moveTo(p);
            if (geo.isValid()) {
                painter->drawImage(p,subSurface->image());
            }
            paintChildren(subSurface,painter,window);
        }
    }
#endif //QT_COMPOSITOR_WAYLAND_GL



    void paintEvent(QPaintEvent *) {
        QPainter p(this);

        if (!m_background.isNull())
            p.drawPixmap(rect(), m_backgroundScaled);

#ifdef QT_COMPOSITOR_WAYLAND_GL
        cleanupGraphicsResources();
        if (!m_textureCache) {
            m_textureCache = new QOpenGLTextureCache(context()->contextHandle());
        }
        if (!m_textureBlitter) {
            m_textureBlitter = new TextureBlitter();
        }
        m_textureBlitter->bind();
#endif
        for (int i = 0; i < m_surfaces.size(); ++i) {
#ifdef QT_COMPOSITOR_WAYLAND_GL
            GLuint texture = composeSurface(m_surfaces.at(i));
            QWaylandSurface *surface = m_surfaces.at(i);
            QRect geo(surface->pos().toPoint(),surface->size());
            m_textureBlitter->drawTexture(texture,geo,size(),0,false,m_surfaces.at(i)->isYInverted());
#else
            QImage img = composeSurface(m_surfaces.at(i));
            p.drawImage(m_surfaces.at(i)->geometry().topLeft(),img);
#endif //QT_COMPOSITOR_WAYLAND_GL
        }

        frameFinished();

#ifdef QT_COMPOSITOR_WAYLAND_GL
        //jl:FIX FIX FIX:)
//        update();
        m_textureBlitter->release();
#endif
    }

    void resizeEvent(QResizeEvent *)
    {
        if (!m_background.isNull()) {
            m_backgroundScaled = QPixmap::fromImage(m_background.scaled(size(),
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    void raise(QWaylandSurface *surface) {
        defaultInputDevice()->setKeyboardFocus(surface);
        surfaceDamaged(surface, QRect(QPoint(), surface->size()));
        m_surfaces.removeOne(surface);
        m_surfaces.append(surface);
    }

    void mousePressEvent(QMouseEvent *e) {
        m_cursorPos = e->pos();
        QPointF local;
        if (QWaylandSurface *surface = surfaceAt(e->pos(), &local)) {
            raise(surface);
            if (e->modifiers() & Qt::ControlModifier) {
                m_moveSurface = surface;
                m_moveOffset = local;
            } else {
                defaultInputDevice()->sendMousePressEvent(e->button(), local.toPoint(),e->pos());
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *e) {
        m_cursorPos = e->pos();
        if (isDragging()) {
            QPoint global = e->pos(); // "global" here means the window of the compositor
            QPointF local;
            QWaylandSurface *surface = surfaceAt(e->pos(), &local);
            if (surface) {
                if (!m_dragSourceSurface)
                    m_dragSourceSurface = surface;
                if (m_dragSourceSurface == surface)
                    m_lastDragSourcePos = local;
                raise(surface);
            }
            //this should go away when draggin is reimplemented
            sendDragMoveEvent(global, local.toPoint(), surface);
            return;
        }
        if (m_moveSurface) {
            m_moveSurface->setPos(e->localPos() - m_moveOffset);
            update();
            return;
        }
        QPointF local;
        if (surfaceAt(e->pos(), &local))
            defaultInputDevice()->sendMouseMoveEvent(local.toPoint(),pos());
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        if (isDragging()) {
            sendDragEndEvent();
            if (m_dragSourceSurface) {
                // Must send a release event to the source too, no matter where the cursor is now.
                // This is a hack and should go away when we reimplement draging
                defaultInputDevice()->sendMouseReleaseEvent(e->button(), m_lastDragSourcePos.toPoint(), e->pos());
                m_dragSourceSurface = 0;
            }
        }
        if (m_moveSurface) {
            m_moveSurface = 0;
            return;
        }
        QPointF local;
        if (surfaceAt(e->pos(), &local))
            defaultInputDevice()->sendMouseReleaseEvent(e->button(), local.toPoint(), e->pos());
    }

    void keyPressEvent(QKeyEvent *event)
    {
        defaultInputDevice()->sendKeyPressEvent(event->nativeScanCode());
    }

    void keyReleaseEvent(QKeyEvent *event)
    {
        defaultInputDevice()->sendKeyReleaseEvent(event->nativeScanCode());
    }

    QWaylandSurface *surfaceAt(const QPointF &point, QPointF *local = 0) {
        for (int i = m_surfaces.size() - 1; i >= 0; --i) {
            QWaylandSurface *surface = m_surfaces.at(i);
            QRect geo(surface->pos().toPoint(),surface->size());
            if (geo.contains(point.toPoint())) {
                if (local)
                    *local = point - surface->pos();
                return surface;
            }
        }
        return 0;
    }

    void setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY) {
        m_cursorSurface = surface;
        m_cursorHotspot = QPoint(hotspotX, hotspotY);
        update();
    }

private:
    QImage m_background;
    QPixmap m_backgroundScaled;

    QList<QWaylandSurface *> m_surfaces;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint m_surfaceCompositorFbo;
    TextureBlitter *m_textureBlitter;
    QOpenGLTextureCache *m_textureCache;
#endif

    QWaylandSurface *m_moveSurface;
    QPointF m_moveOffset;
    QWaylandSurface *m_dragSourceSurface;
    QPointF m_lastDragSourcePos;

    QWaylandSurface* m_cursorSurface;
    QPoint m_cursorPos;
    QPoint m_cursorHotspot;

    friend class TouchObserver;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidgetCompositor compositor;
    compositor.resize(800, 600);
    compositor.show();

//    QTouchScreenHandlerThread t(QString(), new TouchObserver(&compositor));

    return app.exec();
}

#include "main.moc"
