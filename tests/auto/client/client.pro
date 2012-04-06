CONFIG += testcase
TARGET = tst_client

QT += testlib
QT += core-private gui-private

!contains(QT_CONFIG, no-pkg-config) {
    #If Qt uses pkg-config then override pkgconfig from mkspec
    QMAKE_CFLAGS_WAYLAND=$$system(pkg-config --cflags wayland-client 2>/dev/null)
    QMAKE_LIBS_WAYLAND_CLIENT=$$system(pkg-config --libs-only-l wayland-client 2>/dev/null)
    QMAKE_LIBS_WAYLAND_SERVER=$$system(pkg-config --libs-only-l wayland-server 2>/dev/null)
    QMAKE_INCDIR_WAYLAND=$$system("pkg-config --cflags-only-I wayland-client 2>/dev/null | sed -e 's,^-I,,g' -e 's, -I, ,g'")
    QMAKE_LIBDIR_WAYLAND=$$system("pkg-config --libs-only-L wayland-client 2>/dev/null | sed -e 's,^-L,,g' -e 's, -L, ,g'")
    QMAKE_DEFINES_WAYLAND=""
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND
QMAKE_CFLAGS += $$QMAKE_CFLAGS_WAYLAND
DEFINES += $$QMAKE_DEFINES_WAYLAND
LIBS += $$QMAKE_LIBS_WAYLAND_CLIENT
LIBS += $$QMAKE_LIBS_WAYLAND_SERVER
!isEmpty(QMAKE_LIBDIR_WAYLAND) {
    LIBS += -L$$QMAKE_LIBDIR_WAYLAND
}

!isEmpty(QMAKE_LFLAGS_RPATH) {
    !isEmpty(QMAKE_LIBDIR_WAYLAND) {
        QMAKE_LFLAGS += $${QMAKE_LFLAGS_RPATH}$${QMAKE_LIBS_WAYLAND}
    }
}

SOURCES += tst_client.cpp \
           mockcompositor.cpp \
           mockinput.cpp \
           mockshell.cpp \
           mocksurface.cpp \
           mockoutput.cpp
HEADERS += mockcompositor.h \
           mocksurface.h
