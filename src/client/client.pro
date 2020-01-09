TARGET = QtWaylandClient
MODULE = waylandclient

QT += core-private gui-private
QT_FOR_PRIVATE += service_support-private
QT_PRIVATE += fontdatabase_support-private eventdispatcher_support-private theme_support-private

qtConfig(vulkan) {
    QT_PRIVATE += vulkan_support-private
}

# We have a bunch of C code with casts, so we can't have this option
QMAKE_CXXFLAGS_WARN_ON -= -Wcast-qual

# Prevent gold linker from crashing.
# This started happening when QtPlatformSupport was modularized.
use_gold_linker: CONFIG += no_linker_version_script

CONFIG -= precompile_header
CONFIG += link_pkgconfig wayland-scanner

qtConfig(xkbcommon) {
    QT_FOR_PRIVATE += xkbcommon_support-private
}

qtHaveModule(linuxaccessibility_support-private): \
    QT_PRIVATE += linuxaccessibility_support-private

QMAKE_USE += wayland-client

INCLUDEPATH += $$PWD/../shared

WAYLANDCLIENTSOURCES += \
            ../extensions/surface-extension.xml \
            ../extensions/touch-extension.xml \
            ../extensions/qt-key-unstable-v1.xml \
            ../extensions/qt-windowmanager.xml \
            ../3rdparty/protocol/wp-primary-selection-unstable-v1.xml \
            ../3rdparty/protocol/tablet-unstable-v2.xml \
            ../3rdparty/protocol/text-input-unstable-v2.xml \
            ../3rdparty/protocol/xdg-output-unstable-v1.xml \
            ../3rdparty/protocol/wayland.xml

SOURCES +=  qwaylandintegration.cpp \
            qwaylandnativeinterface.cpp \
            qwaylandshmbackingstore.cpp \
            qwaylandinputdevice.cpp \
            qwaylanddisplay.cpp \
            qwaylandwindow.cpp \
            qwaylandscreen.cpp \
            qwaylandshmwindow.cpp \
            qwaylandshellsurface.cpp \
            qwaylandextendedsurface.cpp \
            qwaylandsubsurface.cpp \
            qwaylandsurface.cpp \
            qwaylandtabletv2.cpp \
            qwaylandtouch.cpp \
            qwaylandqtkey.cpp \
            ../shared/qwaylandmimehelper.cpp \
            ../shared/qwaylandinputmethodeventbuilder.cpp \
            qwaylandabstractdecoration.cpp \
            qwaylanddecorationfactory.cpp \
            qwaylanddecorationplugin.cpp \
            qwaylandwindowmanagerintegration.cpp \
            qwaylandinputcontext.cpp \
            qwaylandshm.cpp \
            qwaylandbuffer.cpp \

HEADERS +=  qwaylandintegration_p.h \
            qwaylandnativeinterface_p.h \
            qwaylanddisplay_p.h \
            qwaylandwindow_p.h \
            qwaylandscreen_p.h \
            qwaylandshmbackingstore_p.h \
            qwaylandinputdevice_p.h \
            qwaylandbuffer_p.h \
            qwaylandshmwindow_p.h \
            qwaylandshellsurface_p.h \
            qwaylandextendedsurface_p.h \
            qwaylandsubsurface_p.h \
            qwaylandsurface_p.h \
            qwaylandtabletv2_p.h \
            qwaylandtouch_p.h \
            qwaylandqtkey_p.h \
            qwaylandabstractdecoration_p.h \
            qwaylanddecorationfactory_p.h \
            qwaylanddecorationplugin_p.h \
            qwaylandwindowmanagerintegration_p.h \
            qwaylandinputcontext_p.h \
            qwaylandshm_p.h \
            qtwaylandclientglobal.h \
            qtwaylandclientglobal_p.h \
            ../shared/qwaylandinputmethodeventbuilder_p.h \
            ../shared/qwaylandmimehelper_p.h \
            ../shared/qwaylandsharedmemoryformathelper_p.h \

qtConfig(clipboard) {
    HEADERS += qwaylandclipboard_p.h
    SOURCES += qwaylandclipboard.cpp
}

include(hardwareintegration/hardwareintegration.pri)
include(shellintegration/shellintegration.pri)
include(inputdeviceintegration/inputdeviceintegration.pri)
include(global/global.pri)

qtConfig(vulkan) {
    HEADERS += \
        qwaylandvulkaninstance_p.h \
        qwaylandvulkanwindow_p.h

    SOURCES += \
        qwaylandvulkaninstance.cpp \
        qwaylandvulkanwindow.cpp
}

qtConfig(cursor) {
    QMAKE_USE += wayland-cursor

    HEADERS += \
        qwaylandcursor_p.h
    SOURCES += \
        qwaylandcursor.cpp
}

qtConfig(wayland-datadevice) {
    HEADERS += \
        qwaylanddatadevice_p.h \
        qwaylanddatadevicemanager_p.h \
        qwaylanddataoffer_p.h \
        qwaylanddatasource_p.h
    SOURCES += \
        qwaylanddatadevice.cpp \
        qwaylanddatadevicemanager.cpp \
        qwaylanddataoffer.cpp \
        qwaylanddatasource.cpp
}

qtConfig(wayland-client-primary-selection) {
    HEADERS += qwaylandprimaryselectionv1_p.h
    SOURCES += qwaylandprimaryselectionv1.cpp
}

qtConfig(draganddrop) {
    HEADERS += \
        qwaylanddnd_p.h
    SOURCES += \
        qwaylanddnd.cpp
}

CONFIG += generated_privates
MODULE_PLUGIN_TYPES = \
            wayland-graphics-integration-client \
            wayland-inputdevice-integration \
            wayland-decoration-client \
            wayland-shell-integration
load(qt_module)
