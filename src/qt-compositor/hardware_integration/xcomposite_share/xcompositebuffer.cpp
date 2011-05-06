#include "xcompositebuffer.h"

XCompositeBuffer::XCompositeBuffer(Wayland::Compositor *compositor, Window window, const QSize &size, struct wl_visual *visual)
    : mWindow(window)
    , mInvertedY(false)
{
    base()->compositor = compositor->base();
    base()->height = size.height();
    base()->width = size.width();
    base()->visual = visual;
}

struct wl_buffer_interface XCompositeBuffer::buffer_interface = {
    XCompositeBuffer::buffer_interface_damage,
    XCompositeBuffer::buffer_interface_destroy
};

void XCompositeBuffer::buffer_interface_damage(struct wl_client *client,
               struct wl_buffer *wl_buffer,
               int x,
               int y,
               int width,
               int height)
{
    Q_UNUSED(client);
    Q_UNUSED(wl_buffer);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void XCompositeBuffer::buffer_interface_destroy(wl_client *client, wl_buffer *buffer)
{
    Q_UNUSED(client);
    Q_UNUSED(buffer);
}

void XCompositeBuffer::delete_resource(struct wl_resource *resource,
                                    struct wl_client *client)
{
    Q_UNUSED(client);
    delete reinterpret_cast<XCompositeBuffer *>(resource);
}

Window XCompositeBuffer::window()
{
    return mWindow;
}
