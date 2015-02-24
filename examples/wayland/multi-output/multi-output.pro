QT += core-private gui-private quick-private compositor-private

SOURCES += \
    main.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/Screen.qml \
    qml/Chrome.qml \
    images/background.jpg \

RESOURCES += multi-output.qrc

CONFIG += link_pkgconfig
PKGCONFIG += wayland-server
LIBS += -lwayland-server


DEFINES += QT_COMPOSITOR_QUICK
