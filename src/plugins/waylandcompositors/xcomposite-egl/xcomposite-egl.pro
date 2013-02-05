PLUGIN_TYPE = waylandcompositors
load(qt_plugin)

QT = compositor compositor-private core-private gui-private

OTHER_FILES += xcomposite-egl.json

include (../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite egl x11
} else {
    LIBS += -lXcomposite -lEGL -lX11
}

HEADERS += \
    xcompositeeglintegration.h

SOURCES += \
    xcompositeeglintegration.cpp \
    main.cpp
