#ifndef XCOMPOSITEHANDLER_H
#define XCOMPOSITEHANDLER_H

#include "wayland_wrapper/wlcompositor.h"

#include "xlibinclude.h"

class XCompositeHandler : public Wayland::Object<struct wl_object>
{
public:
    XCompositeHandler(Wayland::Compositor *compositor, Display *display, QWindow *window);
    void createBuffer(struct wl_client *client, uint32_t id, Window window, const QSize &size, struct wl_visual *visual);

    static void send_root_information(struct wl_client *client, struct wl_object *global, uint32_t version);
    static struct wl_xcomposite_interface xcomposite_interface;

private:
    Wayland::Compositor *mCompositor;
    QWindow *mwindow;
    QWindow *mFakeRootWidget;
    Display *mDisplay;

    static void create_buffer(struct wl_client *client,
                          struct wl_xcomposite *xcomposite,
                          uint32_t id,
                          uint32_t x_window,
                          int width,
                          int height,
                          struct wl_visual *visual);

};

#endif // XCOMPOSITEHANDLER_H
