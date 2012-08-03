#  if you want to compile QtCompositor as part of the application
#  instead of linking to it, remove the QT += compositor and uncomment
#  the following line
#include (../../src/qt-compositor/qt-compositor.pri)

# to make QtCompositor/... style includes working without installing
INCLUDEPATH += $$PWD/../../include

HEADERS += \
            textureblitter.h

SOURCES += \
            main.cpp \
            textureblitter.cpp

QT += core-private gui-private widgets widgets-private opengl opengl-private compositor

RESOURCES += qwidget-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qwidget-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtwayland/qwidget-compositor
INSTALLS += target sources
