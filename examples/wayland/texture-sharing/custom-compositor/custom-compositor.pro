QT += core gui qml

QT += waylandcompositor-private

SOURCES += \
    main.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/Screen.qml \
    images/background.jpg

RESOURCES += compositor.qrc

TARGET = custom-compositor

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/texture-sharing/custom-compositor
INSTALLS += target
