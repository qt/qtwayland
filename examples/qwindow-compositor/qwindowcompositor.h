#ifndef QWINDOWCOMPOSITOR_H
#define QWINDOWCOMPOSITOR_H

#include "waylandcompositor.h"
#include "waylandsurface.h"
#include "textureblitter.h"
#include "qopenglwindow.h"

#include <QtGui/private/qopengltexturecache_p.h>

#include <QObject>

class QWindowCompositor : public QObject, public WaylandCompositor
{
    Q_OBJECT
public:
    QWindowCompositor(QOpenGLWindow *window);
private slots:
    void surfaceDestroyed(QObject *object);
    void surfaceMapped();
    void surfaceDamaged(const QRect &rect);

    void render();
protected:
    void surfaceDamaged(WaylandSurface *surface, const QRect &rect);
    void surfaceCreated(WaylandSurface *surface);

    WaylandSurface* surfaceAt(const QPoint &point, QPoint *local = 0);

    GLuint composeSurface(WaylandSurface *surface);
    void paintChildren(WaylandSurface *surface, WaylandSurface *window);


    bool eventFilter(QObject *obj, QEvent *event);
    QPointF toSurface(WaylandSurface *surface, const QPointF &pos) const;

private:
    QOpenGLWindow *m_window;
    QImage m_backgroundImage;
    GLuint m_backgroundTexture;
    QList<WaylandSurface *> m_surfaces;
    TextureBlitter *m_textureBlitter;
    QOpenGLTextureCache *m_textureCache;
    GLuint m_surface_fbo;
    QTimer m_renderScheduler;

    //Dragging windows around
    WaylandSurface *m_draggingWindow;
    bool m_dragKeyIsPressed;
    QPoint m_drag_diff;

};

#endif // QWINDOWCOMPOSITOR_H
