CONFIG += testcase
TARGET = tst_client

QT += testlib
QT += core-private gui-private

use_pkgconfig {
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
    INCLUDEPATH += $$QMAKE_INCDIR_WAYLAND
    !isEmpty(QMAKE_LIBDIR_WAYLAND) {
        LIBS += -L$$QMAKE_LIBDIR_WAYLAND
    }
    LIBS += -lwayland-server -lffi
}

SOURCES += tst_client.cpp \
           mockcompositor.cpp \
           mockinput.cpp \
           mockshell.cpp \
           mockshm.cpp \
           mocksurface.cpp \
           mockoutput.cpp
HEADERS += mockcompositor.h \
           mockshm.h \
           mocksurface.h
