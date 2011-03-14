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

#include "qtcompositor.h"

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
public:
    QWidgetCompositor() : WaylandCompositor(this), m_dragging(false) {
        setMouseTracking(true);
        m_background = QImage(QLatin1String("background.jpg"));
        //make sure we get the window id and create the glcontext
        //so that clients can successfully initialize egl
        winId();
        if (platformWindow()) {
            platformWindow()->glContext();
        }
    }

protected:

    void surfaceCreated(WaylandSurface *) {
        update();
    }

    void surfaceDestroyed(WaylandSurface *surface) {
        m_surfaces.removeAll(surface);
        if (m_surfaces.isEmpty())
            setInputFocus(0);
        update();
    }

    void surfaceMapped(WaylandSurface *surface, const QRect &rect) {
        QPoint pos;
        int index = m_surfaces.indexOf(surface);
        if (index == -1) {
            uint px = 1 + (qrand() % (width() - rect.width() - 2));
            uint py = 1 + (qrand() % (height() - rect.height() - 2));
            pos = QPoint(px, py);
            surface->setGeometry(QRect(pos, rect.size()));
            m_surfaces.append(surface);
        } else {
            m_surfaces[index]->setGeometry(rect);
        }
        setInputFocus(surface);
        update();
    }

    void surfaceDamaged(WaylandSurface *surface, const QRect &rect) {
        update(rect.translated(surface->geometry().topLeft()));
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

    int raise(int index) {
        setInputFocus(m_surfaces.at(index));
        surfaceDamaged(m_surfaces.at(index), QRect(QPoint(), m_surfaces.at(index)->geometry().size()));
        m_surfaces.append(m_surfaces.takeAt(index));
        return m_surfaces.size() - 1;
    }

    void mousePressEvent(QMouseEvent *e) {
        QPoint local;
        int index = surfaceIndex(e->pos(), &local);
        if (index != -1) {
            index = raise(index);
            if (e->modifiers() & Qt::ControlModifier) {
                m_dragging = true;
                m_dragIndex = index;
                m_dragOffset = local;
            } else {
                setInputFocus(m_surfaces.at(index));
                m_surfaces.at(index)->sendMousePressEvent(local.x(), local.y(), e->button());
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *e) {
        if (m_dragging) {
            m_surfaces[m_dragIndex]->geometry().moveTo(e->pos() - m_dragOffset);
            update();
            return;
        }
        QPoint local;
        int index = surfaceIndex(e->pos(), &local);
        if (index != -1)
            m_surfaces.at(index)->sendMouseMoveEvent(local.x(), local.y());
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        if (m_dragging) {
            m_dragging = false;
            return;
        }
        QPoint local;
        int index = surfaceIndex(e->pos(), &local);
        if (index != -1)
            m_surfaces.at(index)->sendMouseReleaseEvent(local.x(), local.y(), e->button());
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

    int surfaceIndex(const QPoint &point, QPoint *local = 0) {
        for (int i = m_surfaces.size() - 1; i >= 0; --i) {
            if (m_surfaces.at(i)->geometry().contains(point)) {
                if (local)
                    *local = point - m_surfaces.at(i)->geometry().topLeft();
                return i;
            }
        }
        return -1;
    }

private:
    QImage m_background;
    QPixmap m_backgroundScaled;

    QList<WaylandSurface *> m_surfaces;

    bool m_dragging;
    int m_dragIndex;
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

