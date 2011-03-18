/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "waylandcompositor.h"

#include "waylandsurface.h"

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QGLContext>
#include <QGLWidget>
#endif

#include <QDebug>

#ifdef QT_COMPOSITOR_WAYLAND_GL
class QWidgetCompositor : public QGLWidget, public WaylandCompositor
#else
class QWidgetCompositor : public QWidget, public WaylandCompositor
#endif
{
    Q_OBJECT
public:
    QWidgetCompositor() : WaylandCompositor(this), m_dragSurface(0) {
        setMouseTracking(true);
        m_background = QImage(QLatin1String("background.jpg"));
        //make sure we get the window id and create the glcontext
        //so that clients can successfully initialize egl
        winId();
        if (platformWindow()) {
            platformWindow()->glContext();
        }
    }

private slots:
    void surfaceDestroyed(QObject *object) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(object);
        m_surfaces.removeAll(surface);
        if (m_surfaces.isEmpty())
            setInputFocus(0);
        update();
    }

    void surfaceMapped(const QRect &rect) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
        QPoint pos;
        if (!m_surfaces.contains(surface)) {
            uint px = 1 + (qrand() % (width() - rect.width() - 2));
            uint py = 1 + (qrand() % (height() - rect.height() - 2));
            pos = QPoint(px, py);
            surface->setGeometry(QRect(pos, rect.size()));
            m_surfaces.append(surface);
        } else {
            surface->setGeometry(rect);
        }
        setInputFocus(surface);
        update();
    }

    void surfaceDamaged(const QRect &rect) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
        surfaceDamaged(surface, rect);
    }

protected:
    void surfaceDamaged(WaylandSurface *surface, const QRect &rect)
    {
#ifdef QT_COMPOSITOR_WAYLAND_GL
        Q_UNUSED(surface);
        Q_UNUSED(rect);
        update();
#else
        update(rect.translated(surface->geometry().topLeft()));
#endif
    }

    void surfaceCreated(WaylandSurface *surface) {
        connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
        connect(surface, SIGNAL(mapped(const QRect &)), this, SLOT(surfaceMapped(const QRect &)));
        connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
        update();
    }

    void paintEvent(QPaintEvent *) {
        QPainter p(this);

        if (!m_background.isNull())
            p.drawPixmap(rect(), m_backgroundScaled);

        for (int i = 0; i < m_surfaces.size(); ++i) {
            if (m_surfaces.at(i)->type() == WaylandSurface::Texture) {
#ifdef QT_COMPOSITOR_WAYLAND_GL
                QPlatformGLContext *glcontext = platformWindow()->glContext();
                if (glcontext) {
                    QGLContext *context = QGLContext::fromPlatformGLContext(glcontext);
                    context->makeCurrent();
                    context->drawTexture(m_surfaces.at(i)->geometry(),m_surfaces.at(i)->texture());
                }
                break;
#endif //QT_COMPOSITOR_WAYLAND_GL
            } else if (m_surfaces.at(i)->type() == WaylandSurface::Shm) {
                QImage img = m_surfaces.at(i)->image();
                p.drawImage(m_surfaces.at(i)->geometry(), img);
            }
        }

        frameFinished();

#ifdef QT_COMPOSITOR_WAYLAND_GL
        //jl:FIX FIX FIX:)
//        update();
        glFinish();
#endif
    }

    void resizeEvent(QResizeEvent *)
    {
        if (!m_background.isNull()) {
            m_backgroundScaled = QPixmap::fromImage(m_background.scaled(size(),
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    void raise(WaylandSurface *surface) {
        setInputFocus(surface);
        surfaceDamaged(surface, QRect(QPoint(), surface->geometry().size()));
        m_surfaces.removeOne(surface);
        m_surfaces.append(surface);
    }

    void mousePressEvent(QMouseEvent *e) {
        QPoint local;
        if (WaylandSurface *surface = surfaceAt(e->pos(), &local)) {
            raise(surface);
            if (e->modifiers() & Qt::ControlModifier) {
                m_dragSurface = surface;
                m_dragOffset = local;
            } else {
                surface->sendMousePressEvent(local, e->button());
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *e) {
        if (m_dragSurface) {
            QRect geometry = m_dragSurface->geometry();
            geometry.moveTo(e->pos() - m_dragOffset);
            m_dragSurface->setGeometry(geometry);
            update();
            return;
        }
        QPoint local;
        if (WaylandSurface *surface = surfaceAt(e->pos(), &local))
            surface->sendMouseMoveEvent(local);
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        if (m_dragSurface) {
            m_dragSurface = 0;
            return;
        }
        QPoint local;
        if (WaylandSurface *surface = surfaceAt(e->pos(), &local))
            surface->sendMouseReleaseEvent(local, e->button());
    }

    void keyPressEvent(QKeyEvent *event)
    {
        if (m_surfaces.isEmpty())
            return;
        m_surfaces.last()->sendKeyPressEvent(event->nativeScanCode());
    }

    void keyReleaseEvent(QKeyEvent *event)
    {
        if (m_surfaces.isEmpty())
            return;
        m_surfaces.last()->sendKeyReleaseEvent(event->nativeScanCode());
    }

    WaylandSurface *surfaceAt(const QPoint &point, QPoint *local = 0) {
        for (int i = m_surfaces.size() - 1; i >= 0; --i) {
            if (m_surfaces.at(i)->geometry().contains(point)) {
                if (local)
                    *local = point - m_surfaces.at(i)->geometry().topLeft();
                return m_surfaces.at(i);
            }
        }
        return 0;
    }

private:
    QImage m_background;
    QPixmap m_backgroundScaled;

    QList<WaylandSurface *> m_surfaces;

    WaylandSurface *m_dragSurface;
    QPoint m_dragOffset;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidgetCompositor compositor;
    compositor.resize(800, 600);
    compositor.show();

    return app.exec();

}

#include "main.moc"
