QT += waylandclient-private gui-private
CONFIG += wayland-scanner

WAYLANDCLIENTSOURCES += $$PWD/../../../../src/extensions/qt-texture-sharing-unstable-v1.xml

SOURCES += main.cpp \
    $$PWD/../../../../src/imports/texture-sharing/texturesharingextension.cpp

HEADERS += \
    $$PWD/../../../../src/imports/texture-sharing/texturesharingextension.h

INCLUDEPATH += $$PWD/../../../../src/imports/texture-sharing/

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/texture-sharing/cpp-client
INSTALLS += target
