#ifndef DRI2XCBHWINTEGRATION_H
#define DRI2XCBHWINTEGRATION_H

#include "hardware_integration/graphicshardwareintegration.h"

class DrmObject;

class Dri2XcbHWIntegration : public GraphicsHardwareIntegration
{
public:
    Dri2XcbHWIntegration(WaylandCompositor *compositor);

    void initializeHardware(Wayland::Display *waylandDisplay);

    GLuint createTextureFromBuffer(wl_buffer *buffer);

private:
    DrmObject *m_drm_object;
};

#endif // DRI2XCBHWINTEGRATION_H
