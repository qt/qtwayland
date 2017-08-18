CONFIG += testcase link_pkgconfig
QT += testlib

QMAKE_USE += wayland-client wayland-server

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../../../../src/3rdparty/protocol/wayland.xml \
    ../../../../src/3rdparty/protocol/xdg-shell-unstable-v6.xml

INCLUDEPATH += ../shared

SOURCES += \
    ../shared/mockcompositor.cpp \
    ../shared/mockinput.cpp \
    ../shared/mockshell.cpp \
    ../shared/mockxdgshellv6.cpp \
    ../shared/mocksurface.cpp \
    ../shared/mockoutput.cpp

HEADERS += \
    ../shared/mockcompositor.h \
    ../shared/mockinput.h \
    ../shared/mocksurface.h \
    ../shared/mockoutput.h
