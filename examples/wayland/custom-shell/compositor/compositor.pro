QT += core gui qml

QT += waylandcompositor

CONFIG += wayland-scanner
CONFIG += c++11
SOURCES += \
    main.cpp \
    exampleshell.cpp \
    exampleshellintegration.cpp \

HEADERS += \
    exampleshell.h \
    exampleshellintegration.h

OTHER_FILES = \
    qml/main.qml \
    images/background.jpg

WAYLANDSERVERSOURCES += \
            ../protocol/example-shell.xml

RESOURCES += compositor.qrc

TARGET = custom-shell-compositor


target.path = $$[QT_INSTALL_EXAMPLES]/wayland/custom-shell/compositor
INSTALLS += target
