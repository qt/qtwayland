QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client

qtConfig(xkbcommon-evdev): \
    QMAKE_USE += xkbcommon_evdev

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-shell-unstable-v5.xml

HEADERS += \
    qwaylandxdgpopupv5_p.h \
    qwaylandxdgshellv5_p.h \
    qwaylandxdgshellv5integration_p.h \
    qwaylandxdgsurfacev5_p.h \

SOURCES += \
    main.cpp \
    qwaylandxdgpopupv5.cpp \
    qwaylandxdgshellv5.cpp \
    qwaylandxdgshellv5integration.cpp \
    qwaylandxdgsurfacev5.cpp \

OTHER_FILES += \
    xdg-shell-v5.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellV5IntegrationPlugin
load(qt_plugin)
