INCLUDEPATH += $$PWD
use_pkgconfig {
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags wayland-server)
    #for some reason this is not included in the cflags line
    INCLUDEPATH += $$system(pkg-config --variable=includedir wayland-server)
    LIBS += $$system(pkg-config --libs wayland-server)

    #set the rpath
    !isEmpty(QMAKE_LFLAGS_RPATH) {
        WAYLAND_LIBDIR = $$system(pkg-config --variable=libdir wayland-server)
        !isEmpty(WAYLAND_LIBDIR):QMAKE_LFLAGS += $${QMAKE_LFLAGS_RPATH}$${WAYLAND_LIBDIR}
    }
} else {
    INCLUDEPATH += $$PWD/../3rdparty/wayland
    LIBS += -L$$PWD/../../lib
    LIBS += -lwayland-server -lffi
}

wayland_gl {
    system(echo "Qt-Compositor configured with openGL")
    QT += opengl
    DEFINES += QT_COMPOSITOR_WAYLAND_GL
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}

include ($$PWD/util/util.pri)
include ($$PWD/wayland_wrapper/wayland_wrapper.pri)
include ($$PWD/hardware_integration/hardware_integration.pri)
include ($$PWD/compositor_api/compositor_api.pri)
