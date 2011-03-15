#ifndef WAYLANDSURFACE_H
#define WAYLANDSURFACE_H

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtOpenGL/QGLContext>
#endif

class WaylandSurfacePrivate;

namespace Wayland {
class Surface;
}

class WaylandSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurface)
public:
    enum Type {
        Invalid,
        Shm,
        Texture
    };

    WaylandSurface(Wayland::Surface *surface);

    Type type() const;

    void setGeometry(const QRect &geometry);
    QRect geometry() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture() const;
#endif

    void sendMousePressEvent(int x, int y, Qt::MouseButton button);
    void sendMouseReleaseEvent(int x, int y, Qt::MouseButton button);
    void sendMouseMoveEvent(int x, int y);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    Wayland::Surface *handle() const;

signals:
    void mapped(const QRect &rect);
    void damaged(const QRect &rect);

    friend class Wayland::Surface;
};

#endif // WAYLANDSURFACE_H
