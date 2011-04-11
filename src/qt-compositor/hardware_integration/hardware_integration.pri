contains(QT_CONFIG, opengl) {
    HEADERS += \
        $$PWD/graphicshardwareintegration.h

    SOURCES += \
        $$PWD/graphicshardwareintegration.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL
    QT += opengl

    QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)
    isEqual(QT_WAYLAND_GL_CONFIG, custom) {
        QT_WAYLAND_GL_INTEGRATION = $$QT_WAYLAND_GL_CONFIG
    } else {
        contains(QT_CONFIG, opengles2) {
            isEqual(QT_WAYLAND_GL_CONFIG, wayland_egl) {
                QT_WAYLAND_GL_INTEGRATION = $$QT_WAYLAND_GL_CONFIG
                CONFIG += mesa_egl
            } else:isEqual(QT_WAYLAND_GL_CONFIG,dri2_xcb) {
                QT_WAYLAND_GL_INTEGRATION = $$QT_WAYLAND_GL_CONFIG
                CONFIG += dri2_xcb
            } else {
                QT_WAYLAND_GL_INTEGRATION = xcomposite_egl
                CONFIG += xcomposite_egl
            }
        } else {
            QT_WAYLAND_GL_INTEGRATION = xcomposite_glx
            CONFIG += xcomposite_glx
        }
    }
    system(echo "Qt-Compositor configured with openGL: $$QT_WAYLAND_GL_INTEGRATION")
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}

mesa_egl {
    include (mesa_egl/mesa_egl.pri)
}
dri2_xcb {
    include (dri2_xcb/dri2_xcb.pri)
}
xcomposite_egl {
    include (xcomposite_egl/xcomposite_egl.pri)
}
xcomposite_glx {
    include (xcomposite_glx/xcomposite_glx.pri)
}
