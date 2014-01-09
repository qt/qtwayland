PLUGIN_TYPE = waylandcompositors
load(qt_plugin)

QT = compositor compositor-private core-private gui-private

INCLUDEPATH = $$PWD
LIBS += -lwayland-server -lEGL

SOURCES += \
    brcmeglintegration.cpp \
    brcmbuffer.cpp


HEADERS += \
    brcmeglintegration.h \
    brcmbuffer.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/brcm.xml
