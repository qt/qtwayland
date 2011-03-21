INCLUDEPATH += $$PWD
use_pkgconfig {
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags wayland-server)
    #for some reason this is not included in the cflags line
    INCLUDEPATH += $$system(pkg-config --variable=includedir wayland-server)
    LIBS += $$system(pkg-config --libs wayland-server)
} else {
    INCLUDEPATH += $$PWD/../3rdparty/wayland
    LIBS += -L$$PWD/../../lib
    LIBS += -lwayland-server -lffi
}

wayland_gl {
    system(echo "Qt-Compositor configured with openGL")
    QT += opengl
    DEFINES += QT_COMPOSITOR_WAYLAND_GL
    use_pkgconfig {
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags glesv2)
        #for some reason this is not included in the cflags line
        INCLUDEPATH += $$system(pkg-config --variable=includedir glesv2)
        LIBS += $$system(pkg-config --libs glesv2)
    }
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}

include ($$PWD/util/util.pri)
include ($$PWD/wayland_wrapper/wayland_wrapper.pri)
include ($$PWD/hardware_integration/hardware_integration.pri)
include ($$PWD/compositor_api/compositor_api.pri)
