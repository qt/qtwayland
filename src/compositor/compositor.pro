TARGET = QtWaylandCompositor
MODULE = waylandcompositor

QT = core gui-private

qtHaveModule(quick): QT += quick

contains(QT_CONFIG, opengl):MODULE_DEFINES = QT_COMPOSITOR_WAYLAND_GL

CONFIG -= precompile_header
CONFIG += link_pkgconfig

DEFINES += QT_WAYLAND_WINDOWMANAGER_SUPPORT
QMAKE_DOCS = $$PWD/doc/qtwaylandcompositor.qdocconf

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG_PRIVATE += wayland-server
} else {
    LIBS += -lwayland-server
}

INCLUDEPATH += ../shared

HEADERS += ../shared/qwaylandmimehelper_p.h \
           ../shared/qwaylandinputmethodeventbuilder_p.h \
           ../shared/qwaylandshmformathelper_p.h

SOURCES += ../shared/qwaylandmimehelper.cpp \
           ../shared/qwaylandinputmethodeventbuilder.cpp

RESOURCES += compositor.qrc

include ($$PWD/global/global.pri)
include ($$PWD/wayland_wrapper/wayland_wrapper.pri)
include ($$PWD/hardware_integration/hardware_integration.pri)
include ($$PWD/compositor_api/compositor_api.pri)
include ($$PWD/extensions/extensions.pri)

MODULE_PLUGIN_TYPES = \
    wayland-graphics-integration-server
load(qt_module)
