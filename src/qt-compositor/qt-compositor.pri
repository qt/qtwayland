LIBS += -lwayland-server -lffi

wayland_gl {
    QT += opengl
    DEFINES += QT_COMPOSITOR_WAYLAND_GL
}

mesa_egl {
include (mesa_egl/mesa_egl.pri)
DEFINES += QT_COMPOSITOR_MESA_EGL
}

SOURCES += $$PWD/qtcompositor.cpp \
        $$PWD/graphicshardwareintegration.cpp \
        $$PWD/private/wlcompositor.cpp \
        $$PWD/private/wlsurface.cpp \
        $$PWD/private/wloutput.cpp \
        $$PWD/private/wldisplay.cpp \
        $$PWD/private/wlshmbuffer.cpp

HEADERS += $$PWD/qtcompositor.h \
        $$PWD/graphicshardwareintegration.h \
        $$PWD/private/wlcompositor.h \
        $$PWD/private/wlsurface.h \
        $$PWD/private/wloutput.h \
        $$PWD/private/wlshmbuffer.h \
        $$PWD/private/wldisplay.h \
        $$PWD/private/wlobject.h

INCLUDEPATH += $$PWD/../3rdparty/wayland
