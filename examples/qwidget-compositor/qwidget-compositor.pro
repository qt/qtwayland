TEMPLATE = app
TARGET = qwidget-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

#  if you want to compile QtCompositor as part of the application
#  instead of linking to it, remove the QT += compositor and uncomment
#  the following line
#include (../../src/qt-compositor/qt-compositor.pri)


CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

# Touch support
isEmpty(QT_SOURCE_TREE) {
    QTBASE = $$[QT_INSTALL_DATA]
} else {
    QTBASE = $$QT_SOURCE_TREE
}

# Input
HEADERS += \
            textureblitter.h

SOURCES += \
            main.cpp \
            textureblitter.cpp

QT += core-private gui-private widgets widgets-private opengl opengl-private compositor

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qwidget-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor
INSTALLS += target sources
