#include "waylandsurface.h"

#include <private/qobject_p.h>

#include "wayland_wrapper/wlsurface.h"
#include "wayland_wrapper/wlextendedsurface.h"
#include "wayland_wrapper/wlsubsurface.h"

#ifdef QT_COMPOSITOR_QUICK
#include "waylandsurfaceitem.h"
#endif

class WaylandSurfacePrivate : public QObjectPrivate
{
public:
    WaylandSurfacePrivate(Wayland::Surface *srfc)
        : surface(srfc)
#ifdef QT_COMPOSITOR_QUICK
        , surface_item(0)
#endif
    {}

    ~WaylandSurfacePrivate()
    {
#ifdef QT_COMPOSITOR_QUICK
        if (surface_item)
            surface_item->setSurface(0);
#endif
    }

    Wayland::Surface *surface;
#ifdef QT_COMPOSITOR_QUICK
    WaylandSurfaceItem *surface_item;
#endif
};

WaylandSurface::WaylandSurface(Wayland::Surface *surface)
    : QObject(*new WaylandSurfacePrivate(surface))
{

}

WaylandSurface *WaylandSurface::parentSurface() const
{
    Q_D(const WaylandSurface);
    if (d->surface->subSurface()) {
        return d->surface->subSurface()->parent()->waylandSurface();
    }
    return 0;
}

QLinkedList<WaylandSurface *> WaylandSurface::subSurfaces() const
{
    Q_D(const WaylandSurface);
    if (d->surface->subSurface()) {
        return d->surface->subSurface()->subSurfaces();
    }
    return QLinkedList<WaylandSurface *>();
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

bool WaylandSurface::visible() const
{
    Q_D(const WaylandSurface);
    return d->surface->visible();
}

QPointF WaylandSurface::pos() const
{
    Q_D(const WaylandSurface);
    return d->surface->pos();
}

void WaylandSurface::setPos(const QPointF &pos)
{
    Q_D(WaylandSurface);
    d->surface->setPos(pos);
}

QSize WaylandSurface::size() const
{
    Q_D(const WaylandSurface);
    return d->surface->size();
}

void WaylandSurface::setSize(const QSize &size)
{
    Q_D(WaylandSurface);
    d->surface->setSize(size);
}

QImage WaylandSurface::image() const
{
    Q_D(const WaylandSurface);
    return d->surface->image();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint WaylandSurface::texture(QOpenGLContext *context) const
{
    Q_D(const WaylandSurface);
    return d->surface->textureId(context);
}
#else //QT_COMPOSITOR_WAYLAND_GL
uint WaylandSurface::texture(QOpenGLContext *) const
{
    return 0;
}
#endif

Wayland::Surface * WaylandSurface::handle() const
{
    Q_D(const WaylandSurface);
    return d->surface;
}

#ifdef QT_COMPOSITOR_QUICK
WaylandSurfaceItem *WaylandSurface::surfaceItem() const
{
    Q_D(const WaylandSurface);
    return d->surface_item;
}

void WaylandSurface::setSurfaceItem(WaylandSurfaceItem *surfaceItem)
{
    Q_D(WaylandSurface);
    d->surface_item = surfaceItem;
}
#endif //QT_COMPOSITOR_QUICK

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

QVariantMap WaylandSurface::windowProperties() const
{
    Q_D(const WaylandSurface);
    return d->surface->windowProperties();
}

void WaylandSurface::setWindowProperty(const QString &name, const QVariant &value)
{
    Q_D(WaylandSurface);
    d->surface->setWindowProperty(name, value);
}

QPointF WaylandSurface::mapToParent(const QPointF &pos) const
{
    return pos + this->pos();
}

QPointF WaylandSurface::mapTo(WaylandSurface *parent, const QPointF &pos) const
{
    QPointF p = pos;
    if (parent) {
        const WaylandSurface * surface = this;
        while (surface != parent) {
            Q_ASSERT_X(surface, "WaylandSurface::mapTo(WaylandSurface *parent, const QPoint &pos)",
                       "parent must be in parent hierarchy");
            p = surface->mapToParent(p);
            surface = surface->parentSurface();
        }
    }
    return p;

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
