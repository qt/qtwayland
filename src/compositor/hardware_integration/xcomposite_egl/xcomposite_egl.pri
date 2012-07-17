include (../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite egl x11
} else {
    LIBS += -lXcomposite -lEGL -lX11
}

HEADERS += \
    $$PWD/xcompositeeglintegration.h

SOURCES += \
    $$PWD/xcompositeeglintegration.cpp
