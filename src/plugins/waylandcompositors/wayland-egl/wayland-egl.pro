PLUGIN_TYPE = waylandcompositors
load(qt_plugin)

QT = compositor compositor-private core-private gui-private

OTHER_FILES += wayland-egl.json

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../3rdparty/protocol/wayland.xml

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server wayland-egl egl
} else {
    LIBS += -lwayland-egl -lwayland-server -lEGL
}

SOURCES += \
    waylandeglintegration.cpp \
    main.cpp

HEADERS += \
    waylandeglintegration.h
