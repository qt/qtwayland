
!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-egl egl
} else {
    LIBS += -lwayland-egl -lEGL
}

SOURCES += \
    $$PWD/waylandeglintegration.cpp

HEADERS += \
    $$PWD/waylandeglintegration.h
