TEMPLATE = app
TARGET = qwindow-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib
include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp \
    qopenglwindow.cpp \
    surfacerenderer.cpp \
    qwindowcompositor.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

QT += gui opengl

target.path += $$[QT_INSTALL_BINS]
INSTALLS += target

HEADERS += \
    qopenglwindow.h \
    surfacerenderer.h \
    qwindowcompositor.h

RESOURCES += qwindow-compositor.qrc
