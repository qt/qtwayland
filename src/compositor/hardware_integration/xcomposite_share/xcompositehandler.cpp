#include "xcompositehandler.h"

#include "wayland-xcomposite-server-protocol.h"

#include "xcompositebuffer.h"
#include <X11/extensions/Xcomposite.h>

XCompositeHandler::XCompositeHandler(Wayland::Compositor *compositor, Display *display, QWindow *window)
    : mCompositor(compositor)
    , mwindow(window)
    , mDisplay(display)
{
    mCompositor->window()->create();

    mFakeRootWindow = new QWindow(mCompositor->window());
    mFakeRootWindow->setGeometry(QRect(-1,-1,1,1));
    mFakeRootWindow->create();
    mFakeRootWindow->show();
    int composite_event_base, composite_error_base;
    if (XCompositeQueryExtension(mDisplay, &composite_event_base, &composite_error_base)) {

    } else {
        qFatal("XComposite required");
    }
}

void XCompositeHandler::createBuffer(struct wl_client *client, uint32_t id, Window window, const QSize &size)
{
    XCompositeBuffer *buffer = new XCompositeBuffer(mCompositor, window, size);
    Wayland::addClientResource(client,&buffer->base()->resource,
                               id,&wl_buffer_interface,
                               &XCompositeBuffer::buffer_interface,
                               XCompositeBuffer::delete_resource);
}

void XCompositeHandler::xcomposite_bind_func(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    XCompositeHandler *handler = static_cast<XCompositeHandler *>(data);
    wl_resource *resource = wl_client_add_object(client,&wl_xcomposite_interface,&xcomposite_interface,id,handler);
    const char *displayString = XDisplayString(handler->mDisplay);
    wl_resource_post_event(resource, WL_XCOMPOSITE_ROOT, displayString, handler->mFakeRootWindow->winId());
}

void XCompositeHandler::create_buffer(struct wl_client *client,
                      struct wl_resource *xcomposite,
                      uint32_t id,
                      uint32_t x_window,
                      int32_t width,
                      int32_t height)
{
    Window window = (Window)x_window;
    XCompositeHandler *that = reinterpret_cast<XCompositeHandler *>(xcomposite);
    that->createBuffer(client, id, window, QSize(width,height));
}
