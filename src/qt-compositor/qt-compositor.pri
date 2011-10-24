INCLUDEPATH += $$PWD
DEFINES += QT_WAYLAND_WINDOWMANAGER_SUPPORT

!mac:use_pkgconfig {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server

    #set the rpath
    !isEmpty(QMAKE_LFLAGS_RPATH) {
        WAYLAND_NEEDS_RPATH = $$system(pkg-config --libs-only-L wayland-server)
        !isEmpty(WAYLAND_NEEDS_RPATH) {
            WAYLAND_LIBDIR = $$system(pkg-config --variable=libdir wayland-server)
            !isEmpty(WAYLAND_LIBDIR):QMAKE_LFLAGS += $${QMAKE_LFLAGS_RPATH}$${WAYLAND_LIBDIR}
        }
    }
} else {
    INCLUDEPATH += $$PWD/../3rdparty/wayland
    LIBS += -L$$PWD/../../lib
    LIBS += -lwayland-server -lffi
}

include ($$PWD/util/util.pri)
include ($$PWD/wayland_wrapper/wayland_wrapper.pri)
include ($$PWD/hardware_integration/hardware_integration.pri)
include ($$PWD/compositor_api/compositor_api.pri)
include ($$PWD/windowmanagerprotocol/windowmanagerprotocol.pri)
