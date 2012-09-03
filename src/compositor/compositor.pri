CONFIG += link_pkgconfig

DEFINES += QT_WAYLAND_WINDOWMANAGER_SUPPORT

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-server
} else {
    LIBS += -lwayland-server
}

include ($$PWD/global/global.pri)
include ($$PWD/wayland_wrapper/wayland_wrapper.pri)
include ($$PWD/hardware_integration/hardware_integration.pri)
include ($$PWD/compositor_api/compositor_api.pri)
include ($$PWD/windowmanagerprotocol/windowmanagerprotocol.pri)
