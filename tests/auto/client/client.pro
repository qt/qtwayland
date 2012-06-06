CONFIG += testcase link_pkgconfig
TARGET = tst_client

QT += testlib
QT += core-private gui-private

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-server
} else {
    LIBS += -lwayland-client -lwayland-server
}

SOURCES += tst_client.cpp \
           mockcompositor.cpp \
           mockinput.cpp \
           mockshell.cpp \
           mocksurface.cpp \
           mockoutput.cpp
HEADERS += mockcompositor.h \
           mocksurface.h
