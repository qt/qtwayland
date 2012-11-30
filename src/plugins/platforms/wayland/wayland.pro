TARGET = qwayland

PLUGIN_TYPE = platforms
load(qt_plugin)

CONFIG += link_pkgconfig qpa/genericunixfontdatabase

QT += core-private gui-private platformsupport-private

SOURCES =   main.cpp \
            qwaylandintegration.cpp \
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
            qwaylandshell.cpp \
            qwaylandshellsurface.cpp \
            qwaylandextendedoutput.cpp \
            qwaylandextendedsurface.cpp \
            qwaylandsubsurface.cpp \
            qwaylandtouch.cpp \
            qwaylandqtkey.cpp \
            $$PWD/../../../shared/qwaylandmimehelper.cpp \
            qwaylanddecoration.cpp \
            qwaylandshmdecoration.cpp

HEADERS =   qwaylandintegration.h \
            qwaylandnativeinterface.h \
            qwaylandcursor.h \
            qwaylanddisplay.h \
            qwaylandwindow.h \
            qwaylandscreen.h \
            qwaylandshmbackingstore.h \
            qwaylandbuffer.h \
            qwaylandshmwindow.h \
            qwaylandclipboard.h \
            qwaylanddnd.h \
            qwaylanddataoffer.h \
            qwaylanddatadevicemanager.h \
            qwaylanddatasource.h \
            qwaylandshell.h \
            qwaylandshellsurface.h \
            qwaylandextendedoutput.h \
            qwaylandextendedsurface.h \
            qwaylandsubsurface.h \
            qwaylandtouch.h \
            qwaylandqtkey.h \
            $$PWD/../../../shared/qwaylandmimehelper.h \
            qwaylanddecoration.h \
            qwaylandshmdecoration.h

DEFINES += Q_PLATFORM_WAYLAND

config_xkbcommon {
    !contains(QT_CONFIG, no-pkg-config) {
        PKGCONFIG += xkbcommon
    } else {
        LIBS += -lxkbcommon
    }
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}

WAYLANDSOURCES += \
            $$PWD/../../../extensions/surface-extension.xml \
            $$PWD/../../../extensions/sub-surface-extension.xml \
            $$PWD/../../../extensions/output-extension.xml \
            $$PWD/../../../extensions/touch-extension.xml \
            $$PWD/../../../extensions/qtkey-extension.xml


OTHER_FILES += wayland.json

INCLUDEPATH += $$PWD/../../../shared

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

include ($$PWD/gl_integration/gl_integration.pri)
include ($$PWD/windowmanager_integration/windowmanager_integration.pri)

