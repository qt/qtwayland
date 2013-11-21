TARGET = QtWaylandClient
QT += core-private gui-private
QT_FOR_PRIVATE += platformsupport-private

MODULE=waylandclient
load(qt_module)

CONFIG += link_pkgconfig qpa/genericunixfontdatabase wayland-scanner

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

INCLUDEPATH += $$PWD/../shared

WAYLANDCLIENTSOURCES += \
            ../3rdparty/protocol/wayland.xml \
            ../extensions/surface-extension.xml \
            ../extensions/sub-surface-extension.xml \
            ../extensions/output-extension.xml \
            ../extensions/touch-extension.xml \
            ../extensions/qtkey-extension.xml \
            ../extensions/windowmanager.xml \
            ../3rdparty/protocol/text.xml \

SOURCES +=  qwaylandintegration.cpp \
            qwaylandnativeinterface.cpp \
            qwaylandshmbackingstore.cpp \
            qwaylandinputdevice.cpp \
            qwaylandcursor.cpp \
            qwaylanddisplay.cpp \
            qwaylandwindow.cpp \
            qwaylandscreen.cpp \
            qwaylandshmwindow.cpp \
            qwaylandclipboard.cpp \
            qwaylanddnd.cpp \
            qwaylanddataoffer.cpp \
            qwaylanddatadevicemanager.cpp \
            qwaylanddatasource.cpp \
            qwaylandshellsurface.cpp \
            qwaylandextendedoutput.cpp \
            qwaylandextendedsurface.cpp \
            qwaylandsubsurface.cpp \
            qwaylandtouch.cpp \
            qwaylandqtkey.cpp \
            ../shared/qwaylandmimehelper.cpp \
            qwaylanddecoration.cpp \
            qwaylandeventthread.cpp\
            qwaylandwindowmanagerintegration.cpp \
            qwaylandinputcontext.cpp \
            qwaylanddatadevice.cpp \

HEADERS +=  qwaylandintegration.h \
            qwaylandnativeinterface.h \
            qwaylandcursor.h \
            qwaylanddisplay.h \
            qwaylandwindow.h \
            qwaylandscreen.h \
            qwaylandshmbackingstore.h \
            qwaylandinputdevice.h \
            qwaylandbuffer.h \
            qwaylandshmwindow.h \
            qwaylandclipboard.h \
            qwaylanddnd.h \
            qwaylanddataoffer.h \
            qwaylanddatadevicemanager.h \
            qwaylanddatasource.h \
            qwaylandshellsurface.h \
            qwaylandextendedoutput.h \
            qwaylandextendedsurface.h \
            qwaylandsubsurface.h \
            qwaylandtouch.h \
            qwaylandqtkey.h \
            ../shared/qwaylandmimehelper.h \
            qwaylanddecoration.h \
            qwaylandeventthread.h \
            qwaylandwindowmanagerintegration.h \
            qwaylandinputcontext.h \
            qwaylanddatadevice.h \

include(hardwareintegration/hardwareintegration.pri)
