QT += gui qml

SOURCES += \
    main.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/CompositorScreen.qml \
    qml/Chrome.qml \
    qml/Keyboard.qml \
    images/background.jpg \

RESOURCES += fancy-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/fancy-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fancy-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/wayland/fancy-compositor
INSTALLS += target sources
