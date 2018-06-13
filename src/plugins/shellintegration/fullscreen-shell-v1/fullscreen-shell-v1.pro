QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/fullscreen-shell-unstable-v1.xml

HEADERS += \
    qwaylandfullscreenshellv1integration.h \
    qwaylandfullscreenshellv1surface.h

SOURCES += \
    main.cpp \
    qwaylandfullscreenshellv1integration.cpp \
    qwaylandfullscreenshellv1surface.cpp

OTHER_FILES += \
    fullscreen-shell-v1.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandFullScreenShellV1IntegrationPlugin
load(qt_plugin)
