#include "dri2xcbhwintegration.h"

#include "dri2xcbbuffer.h"

#include "wlobject.h"
#include "wldisplay.h"
#include "wlcompositor.h"

#include "wayland-server.h"
#include "wayland-drm-server-protocol.h"

#include <QtCore/QDebug>

#include <xcb/xcb.h>
#include <xcb/dri2.h>

class DrmObject : public Wayland::Object<struct wl_object>
{
public:
    DrmObject(Wayland::Compositor *compositor, QWidget *topLevelWidget)
        :m_compositor(compositor)
    {
        QPlatformNativeInterface *nativeInterface = QApplicationPrivate::platformIntegration()->nativeInterface();
        char *deviceName = static_cast<char *>(nativeInterface->nativeResourceForWidget("GraphicsDevice",topLevelWidget));
        m_device_name = QByteArray(deviceName);

        m_connection = static_cast<xcb_connection_t *>(nativeInterface->nativeResourceForWidget("Connection",topLevelWidget));
        m_egl_display = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWidget("EglDisplay",topLevelWidget));
    }
    QByteArray deviceName()
    {
        return m_device_name;
    }

    void authenticate(struct wl_client *client, uint32_t id)
    {

        xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(xcb_get_setup(m_connection));
        xcb_dri2_authenticate_cookie_t authenticateCoockie = xcb_dri2_authenticate_unchecked(m_connection,screenIterator.data->root,id);
        xcb_dri2_authenticate_reply_t *authenticate = xcb_dri2_authenticate_reply(m_connection,authenticateCoockie,NULL);

        if (authenticate && authenticate->authenticated) {
            wl_client_post_event(client, base(), WL_DRM_AUTHENTICATED);
        } else {
            qDebug() << "Failed to authenticate drm :(";
        }

        delete authenticate;
    }

    void createBuffer(wl_client *client, uint32_t id, uint32_t name, const QSize &size, uint32_t stride, wl_visual *visual)
    {
        Dri2XcbBuffer *buffer = new Dri2XcbBuffer(id,name,size,stride,visual,m_egl_display, m_compositor);
        Wayland::addClientResource(client,&buffer->base()->resource,id,&wl_buffer_interface,&dri2_xcb_buffer_interface,0);
    }

private:
    QByteArray m_device_name;
    xcb_connection_t *m_connection;
    xcb_screen_t *m_screen;
    EGLDisplay m_egl_display;
    Wayland::Compositor *m_compositor;
};

void authenticate(struct wl_client *client,
                     struct wl_drm *drm,
                     uint32_t id)
{
    reinterpret_cast<DrmObject *>(drm)->authenticate(client,id);
}

void create_buffer(struct wl_client *client,
                      struct wl_drm *drm,
                      uint32_t id,
                      uint32_t name,
                      int width,
                      int height,
                      uint32_t stride,
                      struct wl_visual *visual)
{
    DrmObject *drmObject = reinterpret_cast<DrmObject *>(drm);
    drmObject->createBuffer(client,id,name,QSize(width,height),stride,visual);
}

const static struct wl_drm_interface drm_interface = {
        authenticate,
        create_buffer
};

void post_drm_device(struct wl_client *client, struct wl_object *global)
{
    DrmObject *drmObject = Wayland::wayland_cast<DrmObject *>(global);
    qDebug() << drmObject->deviceName().constData();
    wl_client_post_event(client, global, WL_DRM_DEVICE, drmObject->deviceName().constData());
}

Dri2XcbHWIntegration::Dri2XcbHWIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
    , m_drm_object(0)
{
}

void Dri2XcbHWIntegration::initializeHardware(Wayland::Display *waylandDisplay)
{
    //we need a winId now.
    m_compositor->topLevelWidget()->winId();


    m_drm_object = new DrmObject(m_compositor->handle(),m_compositor->topLevelWidget());

    waylandDisplay->addGlobalObject(m_drm_object->base(),&wl_drm_interface,&drm_interface,post_drm_device);
}

GLuint Dri2XcbHWIntegration::createTextureFromBuffer(wl_buffer *buffer)
{
    Dri2XcbBuffer *dri2Buffer = Wayland::wayland_cast<Dri2XcbBuffer *>(buffer);

    GLuint textureId = 0;
    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, dri2Buffer->image());

    return textureId;
}
