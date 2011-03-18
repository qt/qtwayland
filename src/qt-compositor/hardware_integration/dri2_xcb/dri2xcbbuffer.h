#ifndef DRI2XCBBUFFER_H
#define DRI2XCBBUFFER_H

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"

#include <wayland-server.h>

#include <QtCore/QSize>
#include <QtCore/QTextStream>
#include <QtGui/private/qapplication_p.h>
#include <QtGui/QPlatformNativeInterface>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

class Dri2XcbBuffer : public Wayland::Object<struct wl_buffer>
{
public:
    Dri2XcbBuffer(uint32_t id, uint32_t name, const QSize &size, uint32_t stride,
                  wl_visual *visual, EGLDisplay eglDisplay, Wayland::Compositor *compositor);
    ~Dri2XcbBuffer();

    EGLImageKHR image() const;

private:
    EGLImageKHR m_image;
    EGLDisplay m_egl_display;
};

void dri2XcbBufferDestroy(struct wl_client *client, struct wl_buffer *buffer);

const static struct wl_buffer_interface dri2_xcb_buffer_interface = {
    dri2XcbBufferDestroy
};

#endif // DRI2XCBBUFFER_H
