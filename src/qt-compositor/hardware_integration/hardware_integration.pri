HEADERS += \
    $$PWD/graphicshardwareintegration.h

SOURCES += \
    $$PWD/graphicshardwareintegration.cpp

wayland_gl {
    mesa_egl {
        include (mesa_egl/mesa_egl.pri)
        DEFINES += QT_COMPOSITOR_MESA_EGL
    }

    dri2_xcb {
        include (dri2_xcb/dri2_xcb.pri)
        DEFINES += QT_COMPOSITOR_DRI2_XCB
    }
}
