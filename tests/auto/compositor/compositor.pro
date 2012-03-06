CONFIG += testcase
TARGET = tst_compositor

QT += testlib
QT += core-private gui-private compositor

INCLUDEPATH += $$QMAKE_INCDIR_WAYLAND

LIBS += $$QMAKE_LIBS_WAYLAND
mac {
    LIBS += -lwayland-client
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND

SOURCES += tst_compositor.cpp testcompositor.cpp mockclient.cpp
HEADERS += testcompositor.h mockclient.h
