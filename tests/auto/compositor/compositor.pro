CONFIG += testcase link_pkgconfig
CONFIG += wayland-scanner
TARGET = tst_compositor

QT += testlib
QT += core-private gui-private waylandcompositor waylandcompositor-private

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-server
} else {
    LIBS += -lwayland-client -lwayland-server
}

config_xkbcommon {
    !contains(QT_CONFIG, no-pkg-config) {
        PKGCONFIG_PRIVATE += xkbcommon
    } else {
        LIBS_PRIVATE += -lxkbcommon
    }
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}

WAYLANDCLIENTSOURCES += \
            ../../../src/3rdparty/protocol/xdg-shell.xml \

SOURCES += tst_compositor.cpp \
           testcompositor.cpp \
           testkeyboardgrabber.cpp \
           mockclient.cpp \
           mockseat.cpp \
           testinputdevice.cpp

HEADERS += testcompositor.h \
           testkeyboardgrabber.h \
           mockclient.h \
           mockseat.h \
           testinputdevice.h
