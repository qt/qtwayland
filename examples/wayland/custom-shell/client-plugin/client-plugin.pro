QT += gui-private waylandclient-private
CONFIG += plugin wayland-scanner
TEMPLATE = lib

QMAKE_USE += wayland-client

qtConfig(xkbcommon): \
    QMAKE_USE += xkbcommon

WAYLANDCLIENTSOURCES += \
    ../protocol/example-shell.xml

HEADERS += \
    exampleshellintegration.h \
    examplesurface.h

SOURCES += \
    main.cpp \
    exampleshellintegration.cpp \
    examplesurface.cpp

OTHER_FILES += \
    example-shell.json

DESTDIR = ../plugins/wayland-shell-integration
TARGET = $$qtLibraryTarget(exampleshellplugin)

### Everything below this line is just to make the example work inside the Qt source tree.
### Do not include the following lines in your own code.

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/customshell/plugins/wayland-shell-integration
INSTALLS += target
CONFIG += install_ok
