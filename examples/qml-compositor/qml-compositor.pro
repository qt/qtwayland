TEMPLATE = app
TARGET = qml-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

LIBS += -L ../../lib

QT += quick v8
QT += quick-private

QT += compositor
#include (../../src/compositor/compositor.pri)

# Input
SOURCES += main.cpp
RESOURCES = qml-compositor.qrc

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qml-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qml-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qml-compositor
INSTALLS += target sources
