TEMPLATE = app
TARGET = qml-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib

QT += declarative v8
QT += opengl

# to be removed once scenegraph gets rid of its widget dependencies
#QT += widgets widgets-private

!isEmpty(QT.core.MAJOR_VERSION):greaterThan(QT.core.MAJOR_VERSION, 4) {
    QT += core-private gui-private declarative-private opengl-private
}

include (../../src/qt-compositor/qt-compositor.pri)

target.path += $$[QT_INSTALL_BINS]
INSTALLS += target

# Input
SOURCES += main.cpp
RESOURCES = qml-compositor.qrc

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
