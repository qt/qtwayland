#ifndef XCOMPOSITEHANDLER_H
#define XCOMPOSITEHANDLER_H

#include "wayland_wrapper/wlcompositor.h"

#include "xlibinclude.h"

class XCompositeHandler : public Wayland::Object<struct wl_object>
{
public:
    XCompositeHandler(Wayland::Compositor *compositor, Display *display, QWindow *window);
    void createBuffer(struct wl_client *client, uint32_t id, Window window, const QSize &size);

    static void xcomposite_bind_func(struct wl_client *client, void *data, uint32_t version, uint32_t id);
    static struct wl_xcomposite_interface xcomposite_interface;

private:
    Wayland::Compositor *mCompositor;
    QWindow *mwindow;
    QWindow *mFakeRootWindow;
    Display *mDisplay;

    static void create_buffer(struct wl_client *client,
                          struct wl_resource *xcomposite,
                          uint32_t id,
                          uint32_t x_window,
                          int32_t width,
                          int32_t height);

};

#endif // XCOMPOSITEHANDLER_H
