TEMPLATE = app
TARGET = client
INCLUDEPATH += .

QT += waylandclient-private

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += ../share-buffer.xml

SOURCES += \
    main.cpp \
    serverbufferrenderer.cpp

HEADERS += \
    serverbufferrenderer.h
