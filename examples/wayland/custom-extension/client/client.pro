PLUGIN_TYPE = platforms
load(qt_plugin)

CONFIG += wayland-scanner

TARGET = custom-wayland

QT += waylandclient-private

WAYLANDCLIENTSOURCES += ../protocol/custom.xml

OTHER_FILES += client.json

SOURCES += main.cpp \
           customextension.cpp

HEADERS += customextension.h

