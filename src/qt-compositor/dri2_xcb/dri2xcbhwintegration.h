#ifndef DRI2XCBHWINTEGRATION_H
#define DRI2XCBHWINTEGRATION_H

#include "../graphicshardwareintegration.h"

class DrmObject;

class Dri2XcbHWIntegration : public GraphicsHardwareIntegration
{
public:
    Dri2XcbHWIntegration(WaylandCompositor *compositor);

    void initializeHardware(Wayland::Display *waylandDisplay);

    void bindBufferToTexture(wl_buffer *buffer, GLuint textureId);

private:
    DrmObject *m_drm_object;
};

#endif // DRI2XCBHWINTEGRATION_H
