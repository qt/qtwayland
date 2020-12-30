INCLUDEPATH += $$PWD

QMAKE_USE += egl wayland-client wayland-egl libdl

QT += opengl-private

SOURCES += $$PWD/qwaylandeglclientbufferintegration.cpp \
           $$PWD/qwaylandglcontext.cpp \
           $$PWD/qwaylandeglwindow.cpp

HEADERS += $$PWD/qwaylandeglclientbufferintegration_p.h \
           $$PWD/qwaylandglcontext_p.h \
           $$PWD/qwaylandeglwindow_p.h \
           $$PWD/qwaylandeglinclude_p.h
