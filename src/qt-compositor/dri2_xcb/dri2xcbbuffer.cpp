#include "dri2xcbbuffer.h"

#include "wlobject.h"

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

Dri2XcbBuffer::Dri2XcbBuffer(uint32_t id, uint32_t name, const QSize &size, uint32_t stride, wl_visual *visual, EGLDisplay eglDisplay, Wayland::Compositor *compositor)
    :m_egl_display(eglDisplay)
{
    base()->compositor = compositor->base();
    base()->visual = visual;
    base()->height = size.height();
    base()->width = size.width();
    base()->attach = 0;
    base()->damage = 0;

    EGLint attribs[] = {
        EGL_WIDTH,                      size.width(),
        EGL_HEIGHT,                     size.height(),
        EGL_DRM_BUFFER_STRIDE_MESA,     stride /4,
        EGL_DRM_BUFFER_FORMAT_MESA,     EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
        EGL_NONE
    };

    m_image = eglCreateImageKHR(m_egl_display,
                              EGL_NO_CONTEXT,
                              EGL_DRM_BUFFER_MESA,
                              (EGLClientBuffer) name, attribs);
}

Dri2XcbBuffer::~Dri2XcbBuffer()
{
    eglDestroyImageKHR (m_egl_display, m_image);
}

void dri2XcbBufferDestroy(struct wl_client *client, struct wl_buffer *buffer)
{
    delete Wayland::wayland_cast<Dri2XcbBuffer *>(buffer);
}

EGLImageKHR Dri2XcbBuffer::image() const
{
    return m_image;
}
