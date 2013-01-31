PLUGIN_TYPE = platforms
load(qt_plugin)

include(../wayland_common/wayland_common.pri)
include (../xcomposite_share/xcomposite_share.pri)

OTHER_FILES += qwayland-xcomposite-glx.json

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite gl x11
} else {
    LIBS += -lXcomposite -lGL -lX11
}

SOURCES += \
    qwaylandxcompositeglxcontext.cpp \
    qwaylandxcompositeglxintegration.cpp \
    qwaylandxcompositeglxwindow.cpp \
    main.cpp

HEADERS += \
    qwaylandxcompositeglxcontext.h \
    qwaylandxcompositeglxintegration.h \
    qwaylandxcompositeglxwindow.h
