CONFIG += testcase
TARGET = tst_compositor

QT += testlib
QT += core-private gui-private compositor

LIBS += -lwayland-client

SOURCES += tst_compositor.cpp testcompositor.cpp mockclient.cpp
HEADERS += testcompositor.h mockclient.h
