QT += gui qml quick quick-private

SOURCES += \
    main.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/CompositorScreen.qml \
    qml/Chrome.qml \
    images/background.jpg \

RESOURCES += qtshell.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/qtshell
sources.path = $$[QT_INSTALL_EXAMPLES]/wayland/qtshell
INSTALLS += target sources
