#include "xcompositebuffer.h"

XCompositeBuffer::XCompositeBuffer(Wayland::Compositor *compositor, Window window, const QSize &size, struct wl_visual *visual)
    : mWindow(window)
{
    base()->attach = 0;
    base()->damage = 0;
    base()->compositor = compositor->base();
    base()->height = size.height();
    base()->width = size.width();
    base()->visual = visual;
}

struct wl_buffer_interface XCompositeBuffer::buffer_interface = {
    XCompositeBuffer::buffer_interface_destroy
};

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
