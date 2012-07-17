!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-egl egl
} else {
    LIBS += -lwayland-egl -lEGL
}

INCLUDEPATH += $$PWD
SOURCES += $$PWD/qwaylandeglintegration.cpp \
           $$PWD/qwaylandglcontext.cpp \
           $$PWD/qwaylandeglwindow.cpp

HEADERS += $$PWD/qwaylandeglintegration.h \
           $$PWD/qwaylandglcontext.h \
           $$PWD/qwaylandeglwindow.h
