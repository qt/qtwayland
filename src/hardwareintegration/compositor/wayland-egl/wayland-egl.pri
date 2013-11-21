INCLUDEPATH += $$PWD

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server wayland-egl egl
} else {
    LIBS += -lwayland-egl -lwayland-server -lEGL
}

SOURCES += \
    $$PWD/waylandeglintegration.cpp

HEADERS += \
    $$PWD/waylandeglintegration.h
