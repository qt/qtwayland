INCLUDEPATH += $$PWD
include($$PWD/../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-client xcomposite x11
} else {
    LIBS += -lXcomposite -lX11
}

CONFIG += egl

SOURCES += \
    $$PWD/qwaylandxcompositeeglcontext.cpp \
    $$PWD/qwaylandxcompositeeglclientbufferintegration.cpp \
    $$PWD/qwaylandxcompositeeglwindow.cpp

HEADERS += \
    $$PWD/qwaylandxcompositeeglcontext.h \
    $$PWD/qwaylandxcompositeeglclientbufferintegration.h \
    $$PWD/qwaylandxcompositeeglwindow.h
