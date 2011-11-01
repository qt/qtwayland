#include "xcompositeeglintegration.h"

#include "waylandobject.h"
#include "wayland_wrapper/wlcompositor.h"
#include "wayland-xcomposite-server-protocol.h"

#include <QtWidgets/QApplication>
#include <QtGui/QPlatformNativeInterface>
#include <QtGui/QPlatformOpenGLContext>

#include "xcompositebuffer.h"
#include "xcompositehandler.h"
#include <X11/extensions/Xcomposite.h>

#include <QtCore/QDebug>

QVector<EGLint> eglbuildSpec()
{
    QVector<EGLint> spec;

    spec.append(EGL_SURFACE_TYPE); spec.append(EGL_WINDOW_BIT | EGL_PIXMAP_BIT);
    spec.append(EGL_RENDERABLE_TYPE); spec.append(EGL_OPENGL_ES2_BIT);
    spec.append(EGL_BIND_TO_TEXTURE_RGBA); spec.append(EGL_TRUE);
    spec.append(EGL_ALPHA_SIZE); spec.append(8);
    spec.append(EGL_NONE);
    return spec;
}


struct wl_xcomposite_interface XCompositeHandler::xcomposite_interface = {
    XCompositeHandler::create_buffer
};

GraphicsHardwareIntegration *GraphicsHardwareIntegration::createGraphicsHardwareIntegration(WaylandCompositor *compositor)
{
    return new XCompositeEglIntegration(compositor);
}

XCompositeEglIntegration::XCompositeEglIntegration(WaylandCompositor *compositor)
    : GraphicsHardwareIntegration(compositor)
    , mDisplay(0)
{
    QPlatformNativeInterface *nativeInterface = QApplication::platformNativeInterface();
    if (nativeInterface) {
        mDisplay = static_cast<Display *>(nativeInterface->nativeResourceForWindow("Display",m_compositor->window()));
        if (!mDisplay)
            qFatal("could not retireve Display from platform integration");
        mEglDisplay = static_cast<EGLDisplay>(nativeInterface->nativeResourceForWindow("EGLDisplay",m_compositor->window()));
        if (!mEglDisplay)
            qFatal("could not retrieve EGLDisplay from plaform integration");
    } else {
        qFatal("Platform integration doesn't have native interface");
    }
    mScreen = XDefaultScreen(mDisplay);
}

void XCompositeEglIntegration::initializeHardware(Wayland::Display *waylandDisplay)
{
    XCompositeHandler *handler = new XCompositeHandler(m_compositor->handle(),mDisplay,m_compositor->window());
    waylandDisplay->addGlobalObject(handler->base(), &wl_xcomposite_interface, &XCompositeHandler::xcomposite_interface,XCompositeHandler::send_root_information);
}

GLuint XCompositeEglIntegration::createTextureFromBuffer(wl_buffer *buffer, QOpenGLContext *)
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    Pixmap pixmap = XCompositeNameWindowPixmap(mDisplay, compositorBuffer->window());

    QVector<EGLint> eglConfigSpec = eglbuildSpec();

    EGLint matching = 0;
    EGLConfig config;
    bool matched = eglChooseConfig(mEglDisplay,eglConfigSpec.constData(),&config,1,&matching);
    if (!matched || !matching) {
        qWarning("Could not retrieve a suitable EGL config");
        return 0;
    }

    QVector<EGLint> attribList;

    attribList.append(EGL_TEXTURE_FORMAT);
    attribList.append(EGL_TEXTURE_RGBA);
    attribList.append(EGL_TEXTURE_TARGET);
    attribList.append(EGL_TEXTURE_2D);
    attribList.append(EGL_NONE);

    EGLSurface surface = eglCreatePixmapSurface(mEglDisplay,config,pixmap,attribList.constData());
    if (surface == EGL_NO_SURFACE) {
        qDebug() << "Failed to create eglsurface" << pixmap << compositorBuffer->window();
    }

    compositorBuffer->setInvertedY(false);

    GLuint textureId;
    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    if (!eglBindTexImage(mEglDisplay,surface,EGL_BACK_BUFFER)) {
        qDebug() << "Failed to bind";
    }

    //    eglDestroySurface(mEglDisplay,surface);

    return textureId;
}

bool XCompositeEglIntegration::isYInverted(wl_buffer *buffer) const
{
    XCompositeBuffer *compositorBuffer = Wayland::wayland_cast<XCompositeBuffer *>(buffer);
    return compositorBuffer->isYInverted();
}
