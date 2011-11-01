#ifndef XCOMPOSITEEGLINTEGRATION_H
#define XCOMPOSITEEGLINTEGRATION_H

#include "hardware_integration/graphicshardwareintegration.h"

#include "xlibinclude.h"

#include <EGL/egl.h>

class XCompositeEglIntegration : public GraphicsHardwareIntegration
{
public:
    XCompositeEglIntegration(WaylandCompositor *compositor);

    void initializeHardware(Wayland::Display *waylandDisplay);

    GLuint createTextureFromBuffer(struct wl_buffer *buffer, QOpenGLContext *context);
    bool isYInverted(wl_buffer *) const;

private:
    Display *mDisplay;
    EGLDisplay mEglDisplay;
    int mScreen;
};

#endif // XCOMPOSITEEGLINTEGRATION_H
