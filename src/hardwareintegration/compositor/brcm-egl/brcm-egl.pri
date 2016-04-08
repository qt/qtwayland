QT = waylandcompositor waylandcompositor-private core-private gui-private

INCLUDEPATH += $$PWD

DEFINES += QT_NO_OPENGL_ES_3

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server
} else {
    LIBS += -lwayland-server
}

for(p, QMAKE_LIBDIR_EGL) {
    exists($$p):LIBS += -L$$p
}

LIBS += $$QMAKE_LIBS_EGL
INCLUDEPATH += $$QMAKE_INCDIR_EGL

SOURCES += \
    $$PWD/brcmeglintegration.cpp \
    $$PWD/brcmbuffer.cpp


HEADERS += \
    $$PWD/brcmeglintegration.h \
    $$PWD/brcmbuffer.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/brcm.xml
