TARGET = qwayland
load(qt_plugin)
load(qt_module)

CONFIG += qpa/genericunixfontdatabase

DESTDIR = $$QT.gui.plugins/platforms

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
            $$PWD/../../../shared/qwaylandmimehelper.cpp

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
            $$PWD/../../../shared/qwaylandmimehelper.h

DEFINES += Q_PLATFORM_WAYLAND

contains(config_test_xkbcommon,yes) {
    !contains(QT_CONFIG, no-pkg-config) {
        QMAKE_CFLAGS_XKBCOMMON=$$system(pkg-config --cflags xkbcommon 2>/dev/null)
        QMAKE_LIBS_XKBCOMMON=$$system(pkg-config --libs xkbcommon 2>/dev/null)
    }

    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_XKBCOMMON
    QMAKE_CFLAGS += $$QMAKE_CFLAGS_XKBCOMMON
    LIBS += $$QMAKE_LIBS_XKBCOMMON
} else {
    DEFINES += QT_NO_WAYLAND_XKB
}

WAYLANDSOURCES += \
            $$PWD/../../../../extensions/surface-extension.xml \
            $$PWD/../../../../extensions/sub-surface-extension.xml \
            $$PWD/../../../../extensions/output-extension.xml \
            $$PWD/../../../../extensions/touch-extension.xml


OTHER_FILES += wayland.json

INCLUDEPATH += $$PWD/../../../shared

!contains(QT_CONFIG, no-pkg-config) {
    #If Qt uses pkg-config then override pkgconfig from mkspec
    QMAKE_CFLAGS_WAYLAND=$$system(pkg-config --cflags wayland-client 2>/dev/null)
    QMAKE_LIBS_WAYLAND_CLIENT=$$system(pkg-config --libs-only-l wayland-client 2>/dev/null)
    QMAKE_INCDIR_WAYLAND=$$system("pkg-config --cflags-only-I wayland-client 2>/dev/null | sed -e 's,^-I,,g' -e 's, -I, ,g'")
    QMAKE_LIBDIR_WAYLAND=$$system("pkg-config --libs-only-L wayland-client 2>/dev/null | sed -e 's,^-L,,g' -e 's, -L, ,g'")
    QMAKE_DEFINES_WAYLAND=""
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WAYLAND
QMAKE_CFLAGS += $$QMAKE_CFLAGS_WAYLAND
DEFINES += $$QMAKE_DEFINES_WAYLAND
LIBS += $$QMAKE_LIBS_WAYLAND_CLIENT
!isEmpty(QMAKE_LIBDIR_WAYLAND) {
    LIBS += -L$$QMAKE_LIBDIR_WAYLAND
}

!isEmpty(QMAKE_LFLAGS_RPATH) {
    !isEmpty(QMAKE_LIBDIR_WAYLAND) {
        QMAKE_LFLAGS += $${QMAKE_LFLAGS_RPATH}$${QMAKE_LIBS_WAYLAND}
    }
}

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target

include ($$PWD/gl_integration/gl_integration.pri)
include ($$PWD/windowmanager_integration/windowmanager_integration.pri)

