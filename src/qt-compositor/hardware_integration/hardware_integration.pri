HEADERS += \
    $$PWD/graphicshardwareintegration.h

SOURCES += \
    $$PWD/graphicshardwareintegration.cpp

wayland_gl{
    contains(QT_CONFIG, opengl) {
        DEFINES += QT_WAYLAND_GL_SUPPORT
        QT += opengl

        contains(QT_CONFIG, opengles2) {
            mesa_egl {
                include (mesa_egl/mesa_egl.pri)
                DEFINES += QT_COMPOSITOR_MESA_EGL
            }
            dri2_xcb {
                include (dri2_xcb/dri2_xcb.pri)
                DEFINES += QT_COMPOSITOR_DRI2_XCB
            }
        } else {
            xcomposite_glx {
                include (xcomposite_glx/xcomposite_glx.pri)
            }
        }
    }
}
