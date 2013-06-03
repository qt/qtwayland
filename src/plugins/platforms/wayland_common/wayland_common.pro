TEMPLATE = lib
CONFIG += staticlib

include ($$PWD/wayland_common_share.pri)
include (windowmanager_integration/windowmanager_integration.pri)

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

WAYLANDCLIENTSOURCES += \
            ../../../extensions/surface-extension.xml \
            ../../../extensions/sub-surface-extension.xml \
            ../../../extensions/output-extension.xml \
            ../../../extensions/touch-extension.xml \
            ../../../extensions/qtkey-extension.xml \

PLUGIN_TYPE = platforms

load(qt_common)
