PLUGIN_TYPE = platforms
load(qt_plugin)

include(../wayland_common/wayland_common.pri)

LIBS += -lEGL

OTHER_FILES += \
    qwayland-brcm-egl.json

SOURCES += qwaylandbrcmeglintegration.cpp \
           qwaylandbrcmglcontext.cpp \
           qwaylandbrcmeglwindow.cpp \
           main.cpp

HEADERS += qwaylandbrcmeglintegration.h \
           qwaylandbrcmglcontext.h \
           qwaylandbrcmeglwindow.h

WAYLANDCLIENTSOURCES += ../../../extensions/brcm.xml
