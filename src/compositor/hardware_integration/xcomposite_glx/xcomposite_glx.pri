include (../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite gl x11
} else {
    LIBS += -lXcomposite -lGL -lX11
}

HEADERS += \
    $$PWD/xcompositeglxintegration.h

SOURCES += \
    $$PWD/xcompositeglxintegration.cpp
