QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client

qtConfig(xkbcommon-evdev): \
    QMAKE_USE += xkbcommon_evdev

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-shell.xml

HEADERS += \
    qwaylandxdgpopup_p.h \
    qwaylandxdgshell_p.h \
    qwaylandxdgshellintegration_p.h \
    qwaylandxdgsurface_p.h \

SOURCES += \
    main.cpp \
    qwaylandxdgpopup.cpp \
    qwaylandxdgshell.cpp \
    qwaylandxdgshellintegration.cpp \
    qwaylandxdgsurface.cpp \

OTHER_FILES += \
    xdg-shell-v5.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellV5IntegrationPlugin
load(qt_plugin)
