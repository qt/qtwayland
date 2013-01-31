PLUGIN_TYPE = platforms
load(qt_plugin)

include(../wayland_common/wayland_common.pri)
include (../xcomposite_share/xcomposite_share.pri)

OTHER_FILES += qwayland-xcomposite-egl.json

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite egl x11
} else {
    LIBS += -lXcomposite -lEGL -lX11
}

SOURCES += \
    qwaylandxcompositeeglcontext.cpp \
    qwaylandxcompositeeglintegration.cpp \
    qwaylandxcompositeeglwindow.cpp \
    main.cpp

HEADERS += \
    qwaylandxcompositeeglcontext.h \
    qwaylandxcompositeeglintegration.h \
    qwaylandxcompositeeglwindow.h
