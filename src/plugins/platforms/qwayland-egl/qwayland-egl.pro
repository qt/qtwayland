PLUGIN_TYPE = platforms
load(qt_plugin)

include(../wayland_common/wayland_common.pri)

OTHER_FILES += \
    qwayland-egl.json

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-egl egl
} else {
    LIBS += -lwayland-egl -lEGL
}

SOURCES += qwaylandeglintegration.cpp \
           qwaylandglcontext.cpp \
           qwaylandeglwindow.cpp \
           main.cpp

HEADERS += qwaylandeglintegration.h \
           qwaylandglcontext.h \
           qwaylandeglwindow.h \
           qwaylandeglinclude.h
