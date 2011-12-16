#include "qwindowcompositor.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>

QWindowCompositor::QWindowCompositor(QOpenGLWindow *window)
    : WaylandCompositor(window)
    , m_window(window)
{
    m_backgroundImage = QImage(QLatin1String(":/background.jpg"));
    m_renderer = new SurfaceRenderer(m_window->context(), m_window);
    m_backgroundTexture = m_renderer->textureFromImage(m_backgroundImage);

    window->installEventFilter(this);

    setRetainedSelectionEnabled(true);

    render();
}

void QWindowCompositor::surfaceDestroyed(QObject *object)
{
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    m_surfaces.removeOne(surface);
    if (inputFocus() == surface || !inputFocus()) // typically reset to 0 already in Compositor::surfaceDestroyed()
        setInputFocus(m_surfaces.isEmpty() ? 0 : m_surfaces.last());
    render();
}

void QWindowCompositor::surfaceMapped(const QSize &size)
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    QPoint pos;
    if (!m_surfaces.contains(surface)) {
        uint px = 1 + (qrand() % (m_window->width() - size.width() - 2));
        uint py = 1 + (qrand() % (m_window->height() - size.height() - 2));
        pos = QPoint(px, py);
        surface->setGeometry(QRect(pos, size));
    } else {
        surface->setGeometry(QRect(window()->geometry().topLeft(),size));
        m_surfaces.removeOne(surface);
    }
    m_surfaces.append(surface);
    setInputFocus(surface);
    render();
}

void QWindowCompositor::surfaceDamaged(const QRect &rect)
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    surfaceDamaged(surface, rect);
}

void QWindowCompositor::surfaceDamaged(WaylandSurface *surface, const QRect &rect)
{
    Q_UNUSED(surface)
    Q_UNUSED(rect)
    render();
}

void QWindowCompositor::surfaceCreated(WaylandSurface *surface)
{
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    render();
}

QPointF QWindowCompositor::toSurface(WaylandSurface *surface, const QPointF &pos) const
{
    return pos - surface->geometry().topLeft();
}

WaylandSurface *QWindowCompositor::surfaceAt(const QPoint &point, QPoint *local)
{
    for (int i = m_surfaces.size() - 1; i >= 0; --i) {
        if (m_surfaces.at(i)->geometry().contains(point)) {
            if (local)
                *local = toSurface(m_surfaces.at(i), point).toPoint();
            return m_surfaces.at(i);
        }
    }
    return 0;
}

void QWindowCompositor::render()
{
    m_window->makeCurrent();

    //Draw the background Image texture
    int w = m_window->width();
    int h = m_window->height();
    int iw = m_backgroundImage.width();
    int ih = m_backgroundImage.height();
    for (int y = 0; y < h; y += ih) {
        for (int x = 0; x < w; x += iw) {
            m_renderer->drawTexture(m_backgroundTexture, QRect(QPoint(x, y), QSize(iw, ih)), 0);
        }
    }

    //Iterate all surfaces in m_surfaces
    //If type == WaylandSurface::Texture draw textureId at geometry
    foreach (WaylandSurface *surface, m_surfaces) {
        if (surface->type() == WaylandSurface::Texture)
            m_renderer->drawTexture(surface->texture(QOpenGLContext::currentContext()), surface->geometry(), 1); //depth argument should be dynamic (focused should be top).
        else if (surface->type() == WaylandSurface::Shm)
            m_renderer->drawImage(surface->image(), surface->geometry());
    }
    frameFinished();
    glFinish();
    m_window->swapBuffers();
    m_window->context()->doneCurrent();
}

bool QWindowCompositor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_window)
        return false;

    switch (event->type()) {
    case QEvent::Expose:
        render();
        break;
    case QEvent::MouseButtonPress: {
        QPoint local;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        WaylandSurface *targetSurface = surfaceAt(me->pos(), &local);
        if (targetSurface) {
            if (inputFocus() != targetSurface) {
                setInputFocus(targetSurface);
                m_surfaces.removeOne(targetSurface);
                m_surfaces.append(targetSurface);
                render();
            }
            targetSurface->sendMousePressEvent(local, me->button());
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        WaylandSurface *targetSurface = inputFocus();
        if (targetSurface) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            targetSurface->sendMouseReleaseEvent(toSurface(targetSurface, me->pos()).toPoint(), me->button());
        }
        break;
    }
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        WaylandSurface *targetSurface = inputFocus();
        if (targetSurface)
            targetSurface->sendKeyPressEvent(ke->nativeScanCode());
        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        WaylandSurface *targetSurface = inputFocus();
        if (targetSurface)
            targetSurface->sendKeyReleaseEvent(ke->nativeScanCode());
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        QSet<WaylandSurface *> targets;
        QTouchEvent *te = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> points = te->touchPoints();
        for (int i = 0; i < points.count(); ++i) {
            const QTouchEvent::TouchPoint &tp(points.at(i));
            QPoint local;
            WaylandSurface *targetSurface = surfaceAt(tp.pos().toPoint(), &local);
            if (targetSurface) {
                targetSurface->sendTouchPointEvent(tp.id(), local.x(), local.y(), tp.state());
                targets.insert(targetSurface);
            }
        }
        foreach (WaylandSurface *surface, targets)
            surface->sendTouchFrameEvent();
        break;
    }
    default:
        break;
    }
    return false;
}
