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

//#include "qtouchscreen.h"
//
//static int touch_x_min, touch_x_max, touch_y_min, touch_y_max;
//
//class QWidgetCompositor;
//
//class TouchObserver : public QTouchScreenObserver
//{
//public:
//    TouchObserver(QWidgetCompositor *compositor)
//        : m_compositor(compositor) { }
//    void touch_configure(int x_min, int x_max, int y_min, int y_max);
//    void touch_point(QEvent::Type state, const QList<QWindowSystemInterface::TouchPoint> &points);
//
//private:
//    QWidgetCompositor *m_compositor;
//};

#ifdef QT_COMPOSITOR_WAYLAND_GL
class QWidgetCompositor : public QGLWidget, public WaylandCompositor
#else
class QWidgetCompositor : public QWidget, public WaylandCompositor
#endif
{
    Q_OBJECT
public:
    QWidgetCompositor() : WaylandCompositor(windowHandle(),const_cast<QGLContext *>(context())), m_moveSurface(0), m_dragSourceSurface(0) {
        setMouseTracking(true);
        setRetainedSelectionEnabled(true);
        m_background = QImage(QLatin1String("background.jpg"));
        //make sure we get the window id and create the glcontext
        //so that clients can successfully initialize egl
        winId();
#ifdef QT_COMPOSITOR_WAYLAND_GL
        if (windowHandle()) {
//            windowHandle()->surfaceHandle();
        }
#endif
    }

private slots:
    void surfaceDestroyed(QObject *object) {
        WaylandSurface *surface = static_cast<WaylandSurface *>(object);
        m_surfaces.removeAll(surface);
        update();
    }

    void surfaceMapped(const QSize &size) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
        QPoint pos;
        if (!m_surfaces.contains(surface)) {
            uint px = 1 + (qrand() % (width() - size.width() - 2));
            uint py = 1 + (qrand() % (height() - size.height() - 2));
            pos = QPoint(px, py);
            surface->setGeometry(QRect(pos, size));
            m_surfaces.append(surface);
        } else {
            surface->setGeometry(QRect(geometry().topLeft(),size));
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
        connect(surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
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
                drawTexture(m_surfaces.at(i)->geometry(), m_surfaces.at(i)->texture());
                break;
#endif //QT_COMPOSITOR_WAYLAND_GL
            } else if (m_surfaces.at(i)->type() == WaylandSurface::Shm) {
                QImage img = m_surfaces.at(i)->image();
                p.drawImage(m_surfaces.at(i)->geometry(), img);
            }
        }

        if (!m_cursor.isNull())
            p.drawImage(m_cursorPos - m_cursorHotspot, m_cursor);

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
        m_cursorPos = e->pos();
        if (!m_cursor.isNull())
            update();
        QPoint local;
        if (WaylandSurface *surface = surfaceAt(e->pos(), &local)) {
            raise(surface);
            if (e->modifiers() & Qt::ControlModifier) {
                m_moveSurface = surface;
                m_moveOffset = local;
            } else {
                surface->sendMousePressEvent(local, e->button());
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *e) {
        m_cursorPos = e->pos();
        if (!m_cursor.isNull())
            update();
        if (isDragging()) {
            QPoint global = e->pos(); // "global" here means the window of the compositor
            QPoint local;
            WaylandSurface *surface = surfaceAt(e->pos(), &local);
            if (surface) {
                if (!m_dragSourceSurface)
                    m_dragSourceSurface = surface;
                if (m_dragSourceSurface == surface)
                    m_lastDragSourcePos = local;
                raise(surface);
            }
            sendDragMoveEvent(global, local, surface);
            return;
        }
        if (m_moveSurface) {
            QRect geometry = m_moveSurface->geometry();
            geometry.moveTo(e->pos() - m_moveOffset);
            m_moveSurface->setGeometry(geometry);
            update();
            return;
        }
        QPoint local;
        if (WaylandSurface *surface = surfaceAt(e->pos(), &local))
            surface->sendMouseMoveEvent(local);
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        if (isDragging()) {
            sendDragEndEvent();
            if (m_dragSourceSurface) {
                // Must send a release event to the source too, no matter where the cursor is now.
                m_dragSourceSurface->sendMouseReleaseEvent(m_lastDragSourcePos, e->button());
                m_dragSourceSurface = 0;
            }
        }
        if (m_moveSurface) {
            m_moveSurface = 0;
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

    void changeCursor(const QImage &image, int hotspotX, int hotspotY) {
        m_cursor = image;
        m_cursorHotspot = QPoint(hotspotX, hotspotY);
        update();
    }

private:
    QImage m_background;
    QPixmap m_backgroundScaled;

    QList<WaylandSurface *> m_surfaces;

    WaylandSurface *m_moveSurface;
    QPoint m_moveOffset;
    WaylandSurface *m_dragSourceSurface;
    QPoint m_lastDragSourcePos;

    QImage m_cursor;
    QPoint m_cursorPos;
    QPoint m_cursorHotspot;

    friend class TouchObserver;
};

//void TouchObserver::touch_configure(int x_min, int x_max, int y_min, int y_max)
//{
//    touch_x_min = x_min;
//    touch_x_max = x_max;
//    touch_y_min = y_min;
//    touch_y_max = y_max;
//}

//void TouchObserver::touch_point(QEvent::Type state, const QList<QWindowSystemInterface::TouchPoint> &points)
//{
//    Q_UNUSED(state);
//    WaylandSurface *focusSurface = m_compositor->inputFocus();
//    if (focusSurface) {
//        if (points.isEmpty())
//            return;
//        for (int i = 0; i < points.count(); ++i) {
//            const QWindowSystemInterface::TouchPoint &point(points.at(i));

//            // These are hw coordinates.
//            int x = int(point.area.left());
//            int y = int(point.area.top());

//            // Wayland expects surface-relative coordinates.

//            // Translate so that (0, 0) is the top-left corner.
//            x = qBound(touch_x_min, x, touch_x_max) - touch_x_min;
//            y = qBound(touch_y_min, y, touch_y_max) - touch_y_min;

//            // Get a normalized position in range 0..1.
//            const int hw_w = touch_x_max - touch_x_min;
//            const int hw_h = touch_y_max - touch_y_min;
//            const qreal nx = x / qreal(hw_w);
//            const qreal ny = y / qreal(hw_h);

//            // Map to surface.
//            QRect winRect(focusSurface->geometry());
//            x = int(nx * winRect.width());
//            y = int(ny * winRect.height());

//            focusSurface->sendTouchPointEvent(point.id,
//                                              x, y,
//                                              point.state);
//        }
//        focusSurface->sendTouchFrameEvent();
//    }
//}

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
