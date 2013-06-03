WAYLANDSERVERSOURCES += \
    ../extensions/surface-extension.xml \
    ../extensions/sub-surface-extension.xml \
    ../extensions/output-extension.xml \
    ../extensions/touch-extension.xml \
    ../extensions/qtkey-extension.xml \
    ../extensions/windowmanager.xml \
    ../3rdparty/protocol/wayland.xml

HEADERS += \
    wayland_wrapper/qwlcompositor_p.h \
    wayland_wrapper/qwldatadevice_p.h \
    wayland_wrapper/qwldatadevicemanager_p.h \
    wayland_wrapper/qwldataoffer_p.h \
    wayland_wrapper/qwldatasource_p.h \
    wayland_wrapper/qwldisplay_p.h \
    wayland_wrapper/qwlextendedoutput_p.h \
    wayland_wrapper/qwlextendedsurface_p.h \
    wayland_wrapper/qwlinputdevice_p.h \
    wayland_wrapper/qwloutput_p.h \
    wayland_wrapper/qwlqtkey_p.h \
    wayland_wrapper/qwlregion_p.h \
    wayland_wrapper/qwlshellsurface_p.h \
    wayland_wrapper/qwlsubsurface_p.h \
    wayland_wrapper/qwlsurface_p.h \
    wayland_wrapper/qwlsurfacebuffer_p.h \
    wayland_wrapper/qwltouch_p.h

SOURCES += \
    wayland_wrapper/qwlcompositor.cpp \
    wayland_wrapper/qwldatadevice.cpp \
    wayland_wrapper/qwldatadevicemanager.cpp \
    wayland_wrapper/qwldataoffer.cpp \
    wayland_wrapper/qwldatasource.cpp \
    wayland_wrapper/qwldisplay.cpp \
    wayland_wrapper/qwlextendedoutput.cpp \
    wayland_wrapper/qwlextendedsurface.cpp \
    wayland_wrapper/qwlinputdevice.cpp \
    wayland_wrapper/qwloutput.cpp \
    wayland_wrapper/qwlqtkey.cpp \
    wayland_wrapper/qwlregion.cpp \
    wayland_wrapper/qwlshellsurface.cpp \
    wayland_wrapper/qwlsubsurface.cpp \
    wayland_wrapper/qwlsurface.cpp \
    wayland_wrapper/qwlsurfacebuffer.cpp \
    wayland_wrapper/qwltouch.cpp

INCLUDEPATH += wayland_wrapper

config_xkbcommon {
    !contains(QT_CONFIG, no-pkg-config) {
        PKGCONFIG += xkbcommon
    } else {
        LIBS += -lxkbcommon
    }
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}
