INCLUDEPATH += $$PWD

QMAKE_USE += xcomposite
CONFIG += wayland-scanner-client-wayland-protocol-include
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/xcomposite.xml

HEADERS += \
    $$PWD/qwaylandxcompositebuffer.h

SOURCES += \
    $$PWD/qwaylandxcompositebuffer.cpp
