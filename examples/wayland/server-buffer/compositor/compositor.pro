QT += core-private gui-private quick-private waylandcompositor-private

contains(QT_CONFIG, no-pkg-config) {
    LIBS += -lwayland-server
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server
}

SOURCES += \
    main.cpp \
    serverbufferitem.cpp

HEADERS += \
    serverbufferitem.h

OTHER_FILES = \
    images/background.jpg

RESOURCES += compositor.qrc

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += ../share-buffer.xml
