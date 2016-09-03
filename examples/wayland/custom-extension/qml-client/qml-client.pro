TEMPLATE = app

QT += qml quick waylandclient-private

CONFIG += c++11
CONFIG += wayland-scanner
CONFIG += link_pkgconfig

WAYLANDCLIENTSOURCES += ../protocol/custom.xml

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

SOURCES += main.cpp \
    ../client-common/customextension.cpp

HEADERS += \
    ../client-common/customextension.h

RESOURCES += qml.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/custom-extension/qml-client
INSTALLS += target


