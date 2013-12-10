QT += core-private gui-private platformsupport-private
CONFIG += link_pkgconfig qpa/genericunixfontdatabase

!equals(QT_WAYLAND_GL_CONFIG, nogl) {
    DEFINES += QT_WAYLAND_GL_SUPPORT
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

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-cursor
    contains(QT_CONFIG, glib): PKGCONFIG_PRIVATE += glib-2.0
} else {
    LIBS += -lwayland-client -lwayland-cursor $$QT_LIBS_GLIB
}

INCLUDEPATH += $$PWD/../../../shared

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += ../../../3rdparty/protocol/wayland.xml
