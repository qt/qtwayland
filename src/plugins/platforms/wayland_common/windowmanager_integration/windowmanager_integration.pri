DEFINES += QT_WAYLAND_WINDOWMANAGER_SUPPORT

contains(DEFINES, QT_WAYLAND_WINDOWMANAGER_SUPPORT) {

    WAYLANDCLIENTSOURCES += $$PWD/../../../../extensions/windowmanager.xml

    HEADERS += \
        $$PWD/qwaylandwindowmanagerintegration.h

    SOURCES += \
        $$PWD/qwaylandwindowmanagerintegration.cpp

}



