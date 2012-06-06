CONFIG += testcase link_pkgconfig
TARGET = tst_compositor

QT += testlib
QT += core-private gui-private compositor

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-server
} else {
    LIBS += -lwayland-client -lwayland-server
}

SOURCES += tst_compositor.cpp testcompositor.cpp mockclient.cpp
HEADERS += testcompositor.h mockclient.h
