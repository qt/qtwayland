CONFIG += testcase link_pkgconfig
QT += testlib
QT += core-private gui-private waylandclient-private

QMAKE_USE += wayland-client wayland-server

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../../../../src/3rdparty/protocol/ivi-application.xml \
    ../../../../src/3rdparty/protocol/wayland.xml \
    ../../../../src/3rdparty/protocol/xdg-shell-unstable-v6.xml \
    ../../../../src/3rdparty/protocol/fullscreen-shell-unstable-v1.xml

INCLUDEPATH += ../shared_old

SOURCES += \
    ../shared_old/mockcompositor.cpp \
    ../shared_old/mockfullscreenshellv1.cpp \
    ../shared_old/mockinput.cpp \
    ../shared_old/mockiviapplication.cpp \
    ../shared_old/mockwlshell.cpp \
    ../shared_old/mockxdgshellv6.cpp \
    ../shared_old/mocksurface.cpp \
    ../shared_old/mockregion.cpp \
    ../shared_old/mockoutput.cpp

HEADERS += \
    ../shared_old/mockcompositor.h \
    ../shared_old/mockfullscreenshellv1.h \
    ../shared_old/mockinput.h \
    ../shared_old/mockiviapplication.h \
    ../shared_old/mockwlshell.h \
    ../shared_old/mockxdgshellv6.h \
    ../shared_old/mocksurface.h \
    ../shared_old/mockregion.h \
    ../shared_old/mockoutput.h
