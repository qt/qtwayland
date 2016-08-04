INCLUDEPATH += $$PWD
!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-client wayland-egl
} else {
    LIBS += -lwayland-client -lwayland-egl
}

DEFINES += QT_EGL_WAYLAND
CONFIG += egl
QT += platformsupport-private

SOURCES += $$PWD/qwaylandeglclientbufferintegration.cpp \
           $$PWD/qwaylandglcontext.cpp \
           $$PWD/qwaylandeglwindow.cpp

HEADERS += $$PWD/qwaylandeglclientbufferintegration.h \
           $$PWD/qwaylandglcontext.h \
           $$PWD/qwaylandeglwindow.h \
           $$PWD/qwaylandeglinclude.h
