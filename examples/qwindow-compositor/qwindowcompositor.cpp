#include "qwindowcompositor.h"

QWindowCompositor::QWindowCompositor(QOpenGLWindow *window)
    : WaylandCompositor(window, window->context())
    , m_window(window)
{
    m_backgroundImage = QImage(QLatin1String("background.jpg"));
    m_renderer = new SurfaceRenderer(m_window->context(), m_window);
    m_backgroundTexture = m_renderer->textureFromImage(m_backgroundImage);
    render();
}

void QWindowCompositor::surfaceDestroyed(QObject *object)
{
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    m_surfaces.removeAll(surface);
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
        m_surfaces.append(surface);
    } else {
        surface->setGeometry(QRect(window()->geometry().topLeft(),size));
    }
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

WaylandSurface * QWindowCompositor::surfaceAt(const QPoint &point, QPoint *local)
{
    for (int i = m_surfaces.size() - 1; i >= 0; --i) {
        if (m_surfaces.at(i)->geometry().contains(point)) {
            if (local)
                *local = point - m_surfaces.at(i)->geometry().topLeft();
            return m_surfaces.at(i);
        }
    }
    return 0;
}

void QWindowCompositor::render()
{
    m_window->makeCurrent();

    //Draw the background Image texture
    m_renderer->drawTexture(m_backgroundTexture, m_window->geometry());

    //Iterate all surfaces in m_surfaces
    //If type == WaylandSurface::Texture draw textureId at geometry
    foreach (WaylandSurface *surface, m_surfaces) {
        if (surface->type() == WaylandSurface::Texture)
            m_renderer->drawTexture(surface->texture(), surface->geometry());
        else if (surface->type() == WaylandSurface::Shm)
            m_renderer->drawImage(surface->image(), surface->geometry());
    }

    frameFinished();
    glFinish();
    m_window->swapBuffers();
    m_window->context()->doneCurrent();
}

