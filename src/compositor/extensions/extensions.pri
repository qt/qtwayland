CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../extensions/surface-extension.xml \
    ../extensions/touch-extension.xml \
    ../extensions/qtkey-extension.xml \
    ../extensions/windowmanager.xml \
    ../3rdparty/protocol/text-input-unstable-v2.xml \
    ../3rdparty/protocol/xdg-shell.xml \

HEADERS += \
    extensions/qwlextendedsurface_p.h \
    extensions/qwlqttouch_p.h \
    extensions/qwlqtkey_p.h \
    extensions/qwaylandwlshell.h \
    extensions/qwaylandwlshell_p.h \
    extensions/qwaylandtextinput.h \
    extensions/qwaylandtextinput_p.h \
    extensions/qwaylandtextinputmanager.h \
    extensions/qwaylandtextinputmanager_p.h \
    extensions/qwaylandwindowmanagerextension.h \
    extensions/qwaylandwindowmanagerextension_p.h \
    extensions/qwaylandxdgshell.h \
    extensions/qwaylandxdgshell_p.h \

SOURCES += \
    extensions/qwlextendedsurface.cpp \
    extensions/qwlqttouch.cpp \
    extensions/qwlqtkey.cpp \
    extensions/qwaylandwlshell.cpp \
    extensions/qwaylandtextinput.cpp \
    extensions/qwaylandtextinputmanager.cpp \
    extensions/qwaylandwindowmanagerextension.cpp \
    extensions/qwaylandxdgshell.cpp \

qtHaveModule(quick) {
    HEADERS += \
        extensions/qwaylandquickwlshellsurfaceitem.h \
        extensions/qwaylandquickwlshellsurfaceitem_p.h \

    SOURCES += \
        extensions/qwaylandquickwlshellsurfaceitem.cpp \

}

INCLUDEPATH += extensions
