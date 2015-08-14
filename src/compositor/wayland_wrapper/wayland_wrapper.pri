CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../3rdparty/protocol/wayland.xml \
    ../3rdparty/protocol/input-method.xml \
    ../3rdparty/protocol/text.xml

HEADERS += \
    wayland_wrapper/qwldatadevice_p.h \
    wayland_wrapper/qwldatadevicemanager_p.h \
    wayland_wrapper/qwldataoffer_p.h \
    wayland_wrapper/qwldatasource_p.h \
    wayland_wrapper/qwlinputmethod_p.h \
    wayland_wrapper/qwlinputmethodcontext_p.h \
    wayland_wrapper/qwlkeyboard_p.h \
    wayland_wrapper/qwlpointer_p.h \
    wayland_wrapper/qwlregion_p.h \
    wayland_wrapper/qwlsurfacebuffer_p.h \
    wayland_wrapper/qwltouch_p.h \
    ../shared/qwaylandxkb.h \

SOURCES += \
    wayland_wrapper/qwldatadevice.cpp \
    wayland_wrapper/qwldatadevicemanager.cpp \
    wayland_wrapper/qwldataoffer.cpp \
    wayland_wrapper/qwldatasource.cpp \
    wayland_wrapper/qwlinputmethod.cpp \
    wayland_wrapper/qwlinputmethodcontext.cpp \
    wayland_wrapper/qwlkeyboard.cpp \
    wayland_wrapper/qwlpointer.cpp \
    wayland_wrapper/qwlregion.cpp \
    wayland_wrapper/qwlsurfacebuffer.cpp \
    wayland_wrapper/qwltouch.cpp \
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
