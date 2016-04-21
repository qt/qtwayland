QT += core-private gui-private quick-private compositor-private

LIBS += -lwayland-server

SOURCES += \
    main.cpp \
    serverbufferitem.cpp
HEADERS += \
    serverbufferitem.h \
    serverbuffertextureprovider.h

OTHER_FILES = \
    qml/main.qml \
    images/background.jpg \

RESOURCES += compositor.qrc

CONFIG +=wayland-scanner
WAYLANDSERVERSOURCES += ../share-buffer.xml

DEFINES += QT_COMPOSITOR_QUICK

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-buffer/compositor
INSTALLS += target
