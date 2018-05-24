QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-shell.xml

HEADERS += \
    qwaylandxdgshell_p.h \
    qwaylandxdgshellintegration_p.h \

SOURCES += \
    main.cpp \
    qwaylandxdgshell.cpp \
    qwaylandxdgshellintegration.cpp \

OTHER_FILES += \
    xdg-shell.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellIntegrationPlugin
load(qt_plugin)
