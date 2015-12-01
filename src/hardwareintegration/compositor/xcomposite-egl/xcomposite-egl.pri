include($$PWD/../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite x11 wayland-server
} else {
    LIBS += -lXcomposite -lX11
}

CONFIG += egl

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/xcompositeeglintegration.h

SOURCES += \
    $$PWD/xcompositeeglintegration.cpp
