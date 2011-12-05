TEMPLATE = app
TARGET = qwindow-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

LIBS += -L ../../lib
#include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp \
    qopenglwindow.cpp \
    surfacerenderer.cpp \
    qwindowcompositor.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

QT += gui

QT += compositor
#include(../../src/compositor/compositor.pri)

HEADERS += \
    qopenglwindow.h \
    surfacerenderer.h \
    qwindowcompositor.h

RESOURCES += qwindow-compositor.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwindow-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qwindow-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwindow-compositor
INSTALLS += target sources
