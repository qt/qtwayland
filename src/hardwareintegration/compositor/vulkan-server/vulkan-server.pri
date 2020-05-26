INCLUDEPATH += $$PWD $$PWD/../../../3rdparty/util

QMAKE_USE_PRIVATE += wayland-server
QT += opengl-private

SOURCES += \
    $$PWD/vulkanserverbufferintegration.cpp \
    $$PWD/vulkanwrapper.cpp

HEADERS += \
    $$PWD/vulkanserverbufferintegration.h \
    $$PWD/vulkanwrapper.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
