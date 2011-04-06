#ifndef XCOMPOSITEBUFFER_H
#define XCOMPOSITEBUFFER_H

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"

#include <QtCore/QSize>

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWidget>

#include <X11/X.h>

class XCompositeBuffer : public Wayland::Object<struct wl_buffer>
{
public:
    XCompositeBuffer(Wayland::Compositor *compositor, Window window, const QSize &size, struct wl_visual *visual);

    Window window();

    static struct wl_buffer_interface buffer_interface;
    static void delete_resource(struct wl_resource *resource,
                                        struct wl_client *client);
private:
    Window mWindow;

    static void buffer_interface_destroy(struct wl_client *client,
                        struct wl_buffer *buffer);
};

#endif // XCOMPOSITORBUFFER_H
