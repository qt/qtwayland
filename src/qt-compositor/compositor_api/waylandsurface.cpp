#include "waylandsurface.h"

#include "wayland_wrapper/wlsurface.h"

class WaylandSurfacePrivate : public QObjectPrivate
{
public:
    WaylandSurfacePrivate(Wayland::Surface *srfc)
        : surface(srfc)
    {}

    Wayland::Surface *surface;
    QRect geometry;
};

WaylandSurface::WaylandSurface(Wayland::Surface *surface)
    : QObject(*new WaylandSurfacePrivate(surface))
{

}

WaylandSurface::Type WaylandSurface::type() const
{
    Q_D(const WaylandSurface);
    return d->surface->type();
}

bool WaylandSurface::isYInverted() const
{
    Q_D(const WaylandSurface);
    return d->surface->isYInverted();
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
#else //QT_COMPOSITOR_WAYLAND_GL
uint WaylandSurface::texture() const
{
    return 0;
}

#endif

Wayland::Surface * WaylandSurface::handle() const
{
    Q_D(const WaylandSurface);
    return d->surface;
}

qint64 WaylandSurface::processId() const
{
    Q_D(const WaylandSurface);
    return d->surface->processId();
}

QByteArray WaylandSurface::authenticationToken() const
{
    Q_D(const WaylandSurface);
    return d->surface->authenticationToken();
}

void WaylandSurface::sendMousePressEvent(const QPoint &pos, Qt::MouseButton button)
{
    Q_D(WaylandSurface);
    d->surface->sendMousePressEvent(pos.x(), pos.y(), button);
}

void WaylandSurface::sendMouseReleaseEvent(const QPoint &pos, Qt::MouseButton button)
{
    Q_D(WaylandSurface);
    d->surface->sendMouseReleaseEvent(pos.x(), pos.y(), button);
}

void WaylandSurface::sendMouseMoveEvent(const QPoint &pos)
{
    Q_D(WaylandSurface);
    d->surface->sendMouseMoveEvent(pos.x(), pos.y());
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

void WaylandSurface::sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state)
{
    Q_D(WaylandSurface);
    d->surface->sendTouchPointEvent(id, x, y, state);
}

void WaylandSurface::sendTouchFrameEvent()
{
    Q_D(WaylandSurface);
    d->surface->sendTouchFrameEvent();
}

void WaylandSurface::sendTouchCancelEvent()
{
    Q_D(WaylandSurface);
    d->surface->sendTouchCancelEvent();
}

void WaylandSurface::frameFinished()
{
    Q_D(WaylandSurface);
    d->surface->frameFinished();
}

void WaylandSurface::setInputFocus()
{
    Q_D(WaylandSurface);
    d->surface->setInputFocus();
}

void WaylandSurface::sendOnScreenVisibilityChange(bool visible)
{
    Q_D(WaylandSurface);
    d->surface->sendOnScreenVisibilityChange(visible);
}
