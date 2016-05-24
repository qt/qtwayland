CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../3rdparty/protocol/wayland.xml \

HEADERS += \
    wayland_wrapper/qwldatadevice_p.h \
    wayland_wrapper/qwldatadevicemanager_p.h \
    wayland_wrapper/qwldataoffer_p.h \
    wayland_wrapper/qwldatasource_p.h \
    wayland_wrapper/qwlregion_p.h \
    wayland_wrapper/qwlsurfacebuffer_p.h \
    ../shared/qwaylandxkb_p.h \

SOURCES += \
    wayland_wrapper/qwldatadevice.cpp \
    wayland_wrapper/qwldatadevicemanager.cpp \
    wayland_wrapper/qwldataoffer.cpp \
    wayland_wrapper/qwldatasource.cpp \
    wayland_wrapper/qwlregion.cpp \
    wayland_wrapper/qwlsurfacebuffer.cpp \
    ../shared/qwaylandxkb.cpp \

INCLUDEPATH += wayland_wrapper

config_xkbcommon {
    !contains(QT_CONFIG, no-pkg-config) {
        PKGCONFIG_PRIVATE += xkbcommon
    } else {
        LIBS_PRIVATE += -lxkbcommon
    }
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}
