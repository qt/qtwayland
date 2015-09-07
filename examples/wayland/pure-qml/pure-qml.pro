QT += core-private gui-private quick-private

SOURCES += \
    main.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/Screen.qml \
    qml/Chrome.qml \
    images/background.jpg \

RESOURCES += pure-qml.qrc

CONFIG += link_pkgconfig
PKGCONFIG += wayland-server
LIBS += -lwayland-server


DEFINES += QT_COMPOSITOR_QUICK
