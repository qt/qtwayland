TEMPLATE = app
TARGET = qwindow-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

LIBS += -L ../../lib
#include (../../src/qt-compositor/qt-compositor.pri)

HEADERS += \
    qopenglwindow.h \
    qwindowcompositor.h \
    textureblitter.h

# Input
SOURCES += main.cpp \
    qopenglwindow.cpp \
    qwindowcompositor.cpp \
    textureblitter.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

QT += gui gui-private core-private

QT += compositor

# to make QtCompositor/... style includes working without installing
INCLUDEPATH += $$PWD/../../include

#  if you want to compile QtCompositor as part of the application
#  instead of linking to it, remove the QT += compositor and uncomment
#  the following line
#include(../../src/compositor/compositor.pri)

RESOURCES += qwindow-compositor.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwindow-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qwindow-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwindow-compositor
INSTALLS += target sources
