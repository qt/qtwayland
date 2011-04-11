qml_folder.source = qml/QmlCompositor
qml_folder.target = qml
DEPLOYMENTFOLDERS = qml_folder

TEMPLATE = app
TARGET = qml-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib

QT += declarative
QT += opengl

include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
