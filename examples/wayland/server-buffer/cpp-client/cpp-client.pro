QT += waylandclient-private gui-private opengl
CONFIG += wayland-scanner

WAYLANDCLIENTSOURCES += ../share-buffer.xml

SOURCES += main.cpp \
    sharebufferextension.cpp

HEADERS += \
    sharebufferextension.h

TARGET = server-buffer-cpp-client

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-buffer/cpp-client
INSTALLS += target
