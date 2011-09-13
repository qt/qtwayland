#ifndef QWINDOWCOMPOSITOR_H
#define QWINDOWCOMPOSITOR_H

#include "waylandcompositor.h"
#include "waylandsurface.h"
#include "surfacerenderer.h"
#include "qopenglwindow.h"

#include <QObject>

class QWindowCompositor : public QObject, public WaylandCompositor
{
    Q_OBJECT
public:
    QWindowCompositor(QOpenGLWindow *window);
private slots:
    void surfaceDestroyed(QObject *object);
    void surfaceMapped(const QSize &size);
    void surfaceDamaged(const QRect &rect);

protected:
    void surfaceDamaged(WaylandSurface *surface, const QRect &rect);
    void surfaceCreated(WaylandSurface *surface);

    WaylandSurface* surfaceAt(const QPoint &point, QPoint *local = 0);

    void render();

private:
    QOpenGLWindow *m_window;
    QImage m_backgroundImage;
    GLuint m_backgroundTexture;
    QList<WaylandSurface *> m_surfaces;
    SurfaceRenderer *m_renderer;
};

#endif // QWINDOWCOMPOSITOR_H
