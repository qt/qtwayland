QT += testlib waylandclient-private
CONFIG += testcase wayland-scanner
QMAKE_USE += wayland-server

WAYLANDSERVERSOURCES += \
    $$PWD/../../../../src/3rdparty/protocol/wayland.xml \
    $$PWD/../../../../src/3rdparty/protocol/xdg-shell.xml

INCLUDEPATH += ../shared

HEADERS += \
    $$PWD/corecompositor.h \
    $$PWD/coreprotocol.h \
    $$PWD/mockcompositor.h \
    $$PWD/xdgshell.h

SOURCES += \
    $$PWD/corecompositor.cpp \
    $$PWD/coreprotocol.cpp \
    $$PWD/mockcompositor.cpp \
    $$PWD/xdgshell.cpp
