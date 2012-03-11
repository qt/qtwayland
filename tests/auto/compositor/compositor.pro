CONFIG += testcase
TARGET = tst_compositor

QT += testlib
QT += core-private gui-private compositor

!contains(QT_CONFIG, no-pkg-config) {
    #If Qt uses pkg-config then override pkgconfig from mkspec
    QMAKE_CFLAGS_WAYLAND=$$system(pkg-config --cflags wayland-server 2>/dev/null)
    QMAKE_LIBS_WAYLAND_SERVER=$$system(pkg-config --libs-only-l wayland-server 2>/dev/null)
    QMAKE_LIBS_WAYLAND_CLIENT=$$system(pkg-config --libs-only-l wayland-client 2>/dev/null)
    QMAKE_INCDIR_WAYLAND=$$system("pkg-config --cflags-only-I wayland-server 2>/dev/null | sed -e 's,^-I,,g' -e 's, -I, ,g'")
    QMAKE_LIBDIR_WAYLAND=$$system("pkg-config --libs-only-L wayland-server 2>/dev/null | sed -e 's,^-L,,g' -e 's, -L, ,g'")
    QMAKE_DEFINES_WAYLAND=""
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND
QMAKE_CFLAGS += $$QMAKE_CFLAGS_WAYLAND
DEFINES += $$QMAKE_DEFINES_WAYLAND
LIBS += $$QMAKE_LIBS_WAYLAND_SERVER
LIBS += $$QMAKE_LIBS_WAYLAND_CLIENT
!isEmpty(QMAKE_LIBDIR_WAYLAND) {
    LIBS += -L$$QMAKE_LIBDIR_WAYLAND
}

!isEmpty(QMAKE_LFLAGS_RPATH) {
    !isEmpty(QMAKE_LIBDIR_WAYLAND) {
        QMAKE_LFLAGS += $${QMAKE_LFLAGS_RPATH}$${QMAKE_LIBS_WAYLAND}
    }
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND

SOURCES += tst_compositor.cpp testcompositor.cpp mockclient.cpp
HEADERS += testcompositor.h mockclient.h
