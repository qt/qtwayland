QT += quick

SOURCES += \
    main.cpp

RESOURCES += \
    qml-client.qrc

DISTFILES += \
    main.qml

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/texture-sharing/qml-client
INSTALLS += target
