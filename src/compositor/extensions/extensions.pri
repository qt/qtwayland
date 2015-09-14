CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../extensions/surface-extension.xml \
    ../extensions/touch-extension.xml \
    ../extensions/qtkey-extension.xml \
    ../extensions/windowmanager.xml \
    ../3rdparty/protocol/input-method.xml \
    ../3rdparty/protocol/text.xml \

HEADERS += \
    extensions/qwlextendedsurface_p.h \
    extensions/qwlqttouch_p.h \
    extensions/qwlqtkey_p.h \
    extensions/qwaylandshell.h \
    extensions/qwaylandshell_p.h \
    extensions/qwaylandwindowmanagerextension.h \
    extensions/qwaylandwindowmanagerextension_p.h \
    extensions/qwltextinput_p.h \
    extensions/qwltextinputmanager_p.h \
    extensions/qwlinputpanel_p.h \
    extensions/qwlinputpanelsurface_p.h \
    extensions/qwlinputmethod_p.h \
    extensions/qwlinputmethodcontext_p.h \

SOURCES += \
    extensions/qwlextendedsurface.cpp \
    extensions/qwlqttouch.cpp \
    extensions/qwlqtkey.cpp \
    extensions/qwaylandshell.cpp \
    extensions/qwaylandwindowmanagerextension.cpp \
    extensions/qwltextinput.cpp \
    extensions/qwltextinputmanager.cpp \
    extensions/qwlinputpanel.cpp \
    extensions/qwlinputpanelsurface.cpp \
    extensions/qwlinputmethod.cpp \
    extensions/qwlinputmethodcontext.cpp \

qtHaveModule(quick) {
    HEADERS += \
        extensions/qwaylandquickshellsurfaceitem.h \
        extensions/qwaylandquickshellsurfaceitem_p.h \

    SOURCES += \
        extensions/qwaylandquickshellsurfaceitem.cpp \

}

INCLUDEPATH += extensions
