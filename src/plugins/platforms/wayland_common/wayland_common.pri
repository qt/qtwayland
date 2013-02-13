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

INCLUDEPATH += $$PWD $$PWD/../../../shared

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client wayland-cursor
} else {
    LIBS += -lwayland-client -lwayland-cursor
}

staticlib = $$shadowed($$PWD)/$${QMAKE_PREFIX_STATICLIB}wayland_common.$${QMAKE_EXTENSION_STATICLIB}
LIBS += $$staticlib
PRE_TARGETDEPS += $$staticlib
