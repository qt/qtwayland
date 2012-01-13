TARGET = qwayland
load(qt_plugin)
load(qt_module)

CONFIG += qpa/genericunixfontdatabase

DESTDIR = $$QT.gui.plugins/platforms

DEFINES += Q_PLATFORM_WAYLAND
DEFINES += $$QMAKE_DEFINES_WAYLAND

mac {
    DEFINES += QT_NO_WAYLAND_XKB
}

QT += core-private gui-private platformsupport-private

WAYLANDSOURCES += \
            $$PWD/../../../../extensions/surface-extension.xml \
            $$PWD/../../../../extensions/sub-surface-extension.xml \
            $$PWD/../../../../extensions/output-extension.xml \
            $$PWD/../../../../extensions/touch-extension.xml

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
            qwaylandtouch.cpp

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
            qwaylandtouch.h

INCLUDEPATH += $$QMAKE_INCDIR_WAYLAND
LIBS += $$QMAKE_LIBS_WAYLAND
mac {
    LIBS += -lwayland-client
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target

include ($$PWD/gl_integration/gl_integration.pri)
include ($$PWD/windowmanager_integration/windowmanager_integration.pri)

