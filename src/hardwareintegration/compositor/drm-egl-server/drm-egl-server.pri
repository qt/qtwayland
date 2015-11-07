INCLUDEPATH += $$PWD

contains(QT_CONFIG, no-pkg-config) {
    LIBS += -lwayland-server
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server
}

CONFIG += egl

SOURCES += \
    $$PWD/drmeglserverbufferintegration.cpp


HEADERS += \
    $$PWD/drmeglserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/drm-egl-server-buffer.xml
