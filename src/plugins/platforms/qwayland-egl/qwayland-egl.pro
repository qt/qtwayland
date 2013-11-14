PLUGIN_TYPE = platforms
load(qt_plugin)

QT += waylandclient-private

OTHER_FILES += \
    qwayland-egl.json

DEFINES += QT_WAYLAND_GL_SUPPORT
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
