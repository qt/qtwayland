INCLUDEPATH += $$PWD

contains(QT_CONFIG, no-pkg-config) {
    LIBS += -lwayland-client
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-client
}

CONFIG += egl

SOURCES += \
        $$PWD/drmeglserverbufferintegration.cpp

HEADERS += \
        $$PWD/drmeglserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/drm-egl-server-buffer.xml
