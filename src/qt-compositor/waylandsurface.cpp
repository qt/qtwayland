#include "waylandsurface.h"

#include "private/wlsurface.h"

class WaylandSurfacePrivate
{
public:
    WaylandSurfacePrivate(Wayland::Surface *srfc)
        : surface(srfc)
    {}

    Wayland::Surface *surface;
    QRect geometry;
};

WaylandSurface::WaylandSurface(Wayland::Surface *surface)
    : d_ptr(new WaylandSurfacePrivate(surface))
{

}

WaylandSurface::Type WaylandSurface::type() const
{
    Q_D(const WaylandSurface);
    return d->surface->type();
}

QRect WaylandSurface::geometry() const
{
    Q_D(const WaylandSurface);
    return d->geometry;
}

void WaylandSurface::setGeometry(const QRect &geometry)
{
    Q_D(WaylandSurface);
    d->geometry = geometry;
}

QImage WaylandSurface::image() const
{
    Q_D(const WaylandSurface);
    return d->surface->image();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint WaylandSurface::texture() const
{
    Q_D(const WaylandSurface);
    return d->surface->textureId();
}
#endif //QT_COMPOSITOR_WAYLAND_GL

Wayland::Surface * WaylandSurface::handle() const
{
    Q_D(const WaylandSurface);
    return d->surface;
}

void WaylandSurface::sendMousePressEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(WaylandSurface);
    d->surface->sendMousePressEvent(x,y,button);
}

void WaylandSurface::sendMouseReleaseEvent(int x, int y, Qt::MouseButton button)
{
    Q_D(WaylandSurface);
    d->surface->sendMouseReleaseEvent(x,y,button);
}

void WaylandSurface::sendMouseMoveEvent(int x, int y)
{
    Q_D(WaylandSurface);
    d->surface->sendMouseMoveEvent(x,y);
}

void WaylandSurface::sendKeyPressEvent(uint code)
{
    Q_D(WaylandSurface);
    d->surface->sendKeyPressEvent(code);
}

void WaylandSurface::sendKeyReleaseEvent(uint code)
{
    Q_D(WaylandSurface);
    d->surface->sendKeyReleaseEvent(code);
}

