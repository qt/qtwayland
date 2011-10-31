#ifndef XCOMPOSITEBUFFER_H
#define XCOMPOSITEBUFFER_H

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"

#include <QtCore/QSize>

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtWidgets/QWidget>

#include <X11/X.h>

class XCompositeBuffer : public Wayland::Object<struct wl_buffer>
{
public:
    XCompositeBuffer(Wayland::Compositor *compositor, Window window, const QSize &size, struct wl_visual *visual);

    Window window();

    static struct wl_buffer_interface buffer_interface;
    static void delete_resource(struct wl_resource *resource);

    bool isYInverted() const { return mInvertedY; }
    void setInvertedY(bool inverted) { mInvertedY = inverted; }
private:
    Window mWindow;
    bool mInvertedY;

    static void buffer_interface_destroy(struct wl_client *client,
                        struct wl_resource *buffer);
    static void buffer_interface_damage(struct wl_client *client,
                   struct wl_resource *buffer,
                   int x,
                   int y,
                   int width,
                   int height);
};

#endif // XCOMPOSITORBUFFER_H
