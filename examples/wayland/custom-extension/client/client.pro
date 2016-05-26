CONFIG += wayland-scanner
CONFIG += link_pkgconfig

TARGET = custom-wayland

QT += waylandclient-private
!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

WAYLANDCLIENTSOURCES += ../protocol/custom.xml

OTHER_FILES += client.json

SOURCES += main.cpp \
           customextension.cpp

HEADERS += customextension.h

PLUGIN_TYPE = platforms
load(qt_plugin)

# Installation into a "proper" Qt location is most unexpected for from an example.
CONFIG += install_ok
