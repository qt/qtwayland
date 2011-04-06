#ifndef XCOMPOSITEGLXINTEGRATION_H
#define XCOMPOSITEGLXINTEGRATION_H

#include "hardware_integration/graphicshardwareintegration.h"

#include "xlibinclude.h"

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

class XCompositeGLXIntegration : public GraphicsHardwareIntegration
{
public:
    XCompositeGLXIntegration(WaylandCompositor *compositor);

    void initializeHardware(Wayland::Display *waylandDisplay);

    GLuint createTextureFromBuffer(struct wl_buffer *buffer);

private:
    PFNGLXBINDTEXIMAGEEXTPROC m_glxBindTexImageEXT;
    PFNGLXRELEASETEXIMAGEEXTPROC m_glxReleaseTexImageEXT;

    Display *mDisplay;
    int mScreen;
};

#endif // XCOMPOSITEGLXINTEGRATION_H
