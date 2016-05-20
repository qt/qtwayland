QT += gui gui-private core-private waylandcompositor waylandcompositor-private

LIBS += -L ../../lib

HEADERS += \
    compositorwindow.h \
    windowcompositor.h

SOURCES += main.cpp \
    compositorwindow.cpp \
    windowcompositor.cpp

# to make QtWaylandCompositor/... style includes working without installing
INCLUDEPATH += $$PWD/../../include


RESOURCES += qwindow-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/qwindow-compositor
INSTALLS += target
