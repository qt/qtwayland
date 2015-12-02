INCLUDEPATH += $$PWD

contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

CONFIG += egl

SOURCES += $$PWD/qwaylandbrcmeglintegration.cpp \
           $$PWD/qwaylandbrcmglcontext.cpp \
           $$PWD/qwaylandbrcmeglwindow.cpp

HEADERS += $$PWD/qwaylandbrcmeglintegration.h \
           $$PWD/qwaylandbrcmglcontext.h \
           $$PWD/qwaylandbrcmeglwindow.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/brcm.xml
