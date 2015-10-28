QT += core gui qml

QT += waylandcompositor-private

CONFIG += wayland-scanner
CONFIG += c++11
SOURCES += \
    main.cpp \
    customextension.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/Screen.qml \
    images/background.jpg

WAYLANDSERVERSOURCES += \
            ../protocol/custom.xml

RESOURCES += compositor.qrc

contains(QT_CONFIG, no-pkg-config) {
    LIBS += -lwayland-server
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server
}

TARGET = custom-compositor

HEADERS += \
    customextension.h

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/custom-extension/compositor
INSTALLS += target
