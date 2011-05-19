TEMPLATE = app
TARGET = qwidget-compositor-mdi
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

!isEmpty(QT.core.MAJOR_VERSION):greaterThan(QT.core.MAJOR_VERSION, 4) {
QT += core-private gui-private
}

DESTDIR=$$PWD/../../bin/

include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
