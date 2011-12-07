TEMPLATE = app
TARGET = qwidget-compositor-mdi
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

#  if you want to compile QtCompositor as part of the application
#  instead of linking to it, remove the QT += compositor and uncomment
#  the following line
#include (../../src/qt-compositor/qt-compositor.pri)

QT += widgets gui-private widgets-private compositor

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor-mdi
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qwidget-compositor-mdi.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor
INSTALLS += target sources
