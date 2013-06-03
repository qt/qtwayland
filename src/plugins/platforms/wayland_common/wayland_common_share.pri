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
} else {
    LIBS += -lwayland-client -lwayland-cursor
}

INCLUDEPATH += $$PWD/../../../shared

WAYLANDCLIENTSOURCES += ../../../3rdparty/protocol/wayland.xml
