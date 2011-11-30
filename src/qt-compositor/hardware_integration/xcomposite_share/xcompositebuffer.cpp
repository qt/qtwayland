#include "xcompositebuffer.h"

XCompositeBuffer::XCompositeBuffer(Wayland::Compositor *compositor, Window window, const QSize &size)
    : mWindow(window)
    , mInvertedY(false)
{
    base()->height = size.height();
    base()->width = size.width();
}

struct wl_buffer_interface XCompositeBuffer::buffer_interface = {
    XCompositeBuffer::buffer_interface_damage,
    XCompositeBuffer::buffer_interface_destroy
};

void XCompositeBuffer::buffer_interface_damage(struct wl_client *client,
               struct wl_resource *buffer,
               int x,
               int y,
               int width,
               int height)
{
    Q_UNUSED(client);
    Q_UNUSED(buffer);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void XCompositeBuffer::buffer_interface_destroy(wl_client *client, wl_resource *buffer)
{
    Q_UNUSED(client);
    Q_UNUSED(buffer);
}

void XCompositeBuffer::delete_resource(struct wl_resource *resource)
{
    delete reinterpret_cast<XCompositeBuffer *>(resource);
}

Window XCompositeBuffer::window()
{
    return mWindow;
}
