TEMPLATE = lib
CONFIG += staticlib

CONFIG += link_pkgconfig qpa/genericunixfontdatabase

QT += core-private gui-private platformsupport-private

include (windowmanager_integration/windowmanager_integration.pri)

!equals(QT_WAYLAND_GL_CONFIG, nogl) {
    DEFINES += QT_WAYLAND_GL_SUPPORT
}

!config_xkbcommon {
    DEFINES += QT_NO_WAYLAND_XKB
}

INCLUDEPATH += ../../../shared

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
            qwaylandshell.cpp \
            qwaylandshellsurface.cpp \
            qwaylandextendedoutput.cpp \
            qwaylandextendedsurface.cpp \
            qwaylandsubsurface.cpp \
            qwaylandtouch.cpp \
            qwaylandqtkey.cpp \
            ../../../shared/qwaylandmimehelper.cpp \
            qwaylanddecoration.cpp \
            qwaylandeventthread.cpp

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
            qwaylandshell.h \
            qwaylandshellsurface.h \
            qwaylandextendedoutput.h \
            qwaylandextendedsurface.h \
            qwaylandsubsurface.h \
            qwaylandtouch.h \
            qwaylandqtkey.h \
            ../../../shared/qwaylandmimehelper.h \
            qwaylanddecoration.h \
            qwaylandeventthread.h

contains(DEFINES, QT_WAYLAND_GL_SUPPORT) {
    SOURCES += qwaylandglintegration.cpp
    HEADERS += qwaylandglintegration.h
}

WAYLANDSOURCES += \
            ../../../extensions/surface-extension.xml \
            ../../../extensions/sub-surface-extension.xml \
            ../../../extensions/output-extension.xml \
            ../../../extensions/touch-extension.xml \
            ../../../extensions/qtkey-extension.xml
