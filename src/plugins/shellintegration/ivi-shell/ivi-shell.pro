PLUGIN_TYPE = wayland-shell-integration
load(qt_plugin)

QT += waylandclient-private
CONFIG += wayland-scanner

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-cursor
    CONFIG += link_pkgconfig
} else {
    LIBS += -lwayland-client -lwayland-cursor
}

config_xkbcommon {
    !contains(QT_CONFIG, no-pkg-config) {
        PKGCONFIG += xkbcommon
    } else {
        LIBS += -lxkbcommon
    }
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/ivi-application.xml \
    ../../../3rdparty/protocol/ivi-controller.xml

HEADERS += \
    qwaylandivishellintegration.h \
    qwaylandivisurface_p.h

SOURCES += \
    main.cpp \
    qwaylandivishellintegration.cpp \
    qwaylandivisurface.cpp

OTHER_FILES += \
    ivi-shell.json
