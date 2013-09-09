PLUGIN_TYPE = waylandcompositors
load(qt_plugin)

QT = compositor compositor-private core-private gui-private

OTHER_FILES += xcomposite-glx.json

include (../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite gl x11 wayland-server
} else {
    LIBS += -lXcomposite -lGL -lX11
}

HEADERS += \
    xcompositeglxintegration.h

SOURCES += \
    xcompositeglxintegration.cpp \
    main.cpp
