#include "xcompositeglxintegration.h"

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"
#include "wayland-xcomposite-server-protocol.h"

#include <QtGui/QPlatformNativeInterface>
#include <QtGui/QOpenGLContext>

#include "xcompositebuffer.h"
#include "xcompositehandler.h"
#include <X11/extensions/Xcomposite.h>

#include <QtCore/QDebug>

QVector<int> qglx_buildSpec()
{
    QVector<int> spec(48);
    int i = 0;

    spec[i++] = GLX_LEVEL;
    spec[i++] = 0;
    spec[i++] = GLX_DRAWABLE_TYPE; spec[i++] = GLX_PIXMAP_BIT | GLX_WINDOW_BIT;
    spec[i++] = GLX_BIND_TO_TEXTURE_TARGETS_EXT; spec[i++] = GLX_TEXTURE_2D_BIT_EXT;
    spec[i++] = GLX_BIND_TO_TEXTURE_RGB_EXT; spec[i++] = TRUE;

    spec[i++] = 0;
    return spec;
}


struct wl_xcomposite_interface XCompositeHandler::xcomposite_interface = {
    XCompositeHandler::create_buffer
};

GraphicsHardwareIntegration *GraphicsHardwareIntegration::createGraphicsHardwareIntegration(WaylandCompositor *compositor)
{
    return new XCompositeGLXIntegration(compositor);
}

XCompositeGLXIntegration::XCompositeGLXIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
    , mDisplay(0)
{
    QPlatformNativeInterface *nativeInterface = QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    if (nativeInterface) {
        mDisplay = static_cast<Display *>(nativeInterface->nativeResourceForWindow("Display",m_compositor->window()));
        if (!mDisplay)
            qFatal("could not retireve Display from platform integration");
    } else {
        qFatal("Platform integration doesn't have native interface");
    }
    mScreen = XDefaultScreen(mDisplay);
}

void XCompositeGLXIntegration::initializeHardware(Wayland::Display *waylandDisplay)
{
    XCompositeHandler *handler = new XCompositeHandler(m_compositor->handle(),mDisplay,m_compositor->window());
    wl_display_add_global(waylandDisplay->handle(),&wl_xcomposite_interface,handler,XCompositeHandler::xcomposite_bind_func);

    QOpenGLContext *glContext = new QOpenGLContext();
    glContext->create();

    m_glxBindTexImageEXT = reinterpret_cast<PFNGLXBINDTEXIMAGEEXTPROC>(glContext->getProcAddress("glXBindTexImageEXT"));
    if (!m_glxBindTexImageEXT) {
        qDebug() << "Did not find glxBindTexImageExt, everything will FAIL!";
    }
    m_glxReleaseTexImageEXT = reinterpret_cast<PFNGLXRELEASETEXIMAGEEXTPROC>(glContext->getProcAddress("glXReleaseTexImageEXT"));
    if (!m_glxReleaseTexImageEXT) {
        qDebug() << "Did not find glxReleaseTexImageExt";
    }

    delete glContext;
}

GLuint XCompositeGLXIntegration::createTextureFromBuffer(wl_buffer *buffer, QOpenGLContext *)
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    Pixmap pixmap = XCompositeNameWindowPixmap(mDisplay, compositorBuffer->window());

    QVector<int> glxConfigSpec = qglx_buildSpec();
    int numberOfConfigs;
    GLXFBConfig *configs = glXChooseFBConfig(mDisplay,mScreen,glxConfigSpec.constData(),&numberOfConfigs);

    QVector<int> attribList;
    attribList.append(GLX_TEXTURE_FORMAT_EXT);
    attribList.append(GLX_TEXTURE_FORMAT_RGB_EXT);
    attribList.append(GLX_TEXTURE_TARGET_EXT);
    attribList.append(GLX_TEXTURE_2D_EXT);
    attribList.append(0);
    GLXPixmap glxPixmap = glXCreatePixmap(mDisplay,*configs,pixmap,attribList.constData());

    uint inverted = 0;
    glXQueryDrawable(mDisplay, glxPixmap, GLX_Y_INVERTED_EXT,&inverted);
    compositorBuffer->setInvertedY(!inverted);

    XFree(configs);

    GLuint textureId;
    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    m_glxBindTexImageEXT(mDisplay,glxPixmap,GLX_FRONT_EXT, 0);
    //Do we need to change the api so that we do bind and release in the painevent?
    //The specification states that when deleting the texture the color buffer is deleted
//    m_glxReleaseTexImageEXT(mDisplay,glxPixmap,GLX_FRONT_EXT);
    return textureId;
}

bool XCompositeGLXIntegration::isYInverted(wl_buffer *buffer) const
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    return compositorBuffer->isYInverted();
}
