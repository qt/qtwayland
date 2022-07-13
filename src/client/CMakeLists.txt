# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from client.pro.

#####################################################################
## WaylandClient Module:
#####################################################################

qt_internal_add_module(WaylandClient
    PLUGIN_TYPES wayland-graphics-integration-client wayland-inputdevice-integration wayland-decoration-client wayland-shell-integration
    GENERATE_CPP_EXPORTS
    SOURCES
        ../shared/qwaylandinputmethodeventbuilder.cpp ../shared/qwaylandinputmethodeventbuilder_p.h
        ../shared/qwaylandmimehelper.cpp ../shared/qwaylandmimehelper_p.h
        ../shared/qwaylandsharedmemoryformathelper_p.h
        global/qwaylandclientextension.cpp global/qwaylandclientextension.h global/qwaylandclientextension_p.h
        hardwareintegration/qwaylandclientbufferintegration.cpp hardwareintegration/qwaylandclientbufferintegration_p.h
        hardwareintegration/qwaylandclientbufferintegrationfactory.cpp hardwareintegration/qwaylandclientbufferintegrationfactory_p.h
        hardwareintegration/qwaylandclientbufferintegrationplugin.cpp hardwareintegration/qwaylandclientbufferintegrationplugin_p.h
        hardwareintegration/qwaylandhardwareintegration.cpp hardwareintegration/qwaylandhardwareintegration_p.h
        hardwareintegration/qwaylandserverbufferintegration.cpp hardwareintegration/qwaylandserverbufferintegration_p.h
        hardwareintegration/qwaylandserverbufferintegrationfactory.cpp hardwareintegration/qwaylandserverbufferintegrationfactory_p.h
        hardwareintegration/qwaylandserverbufferintegrationplugin.cpp hardwareintegration/qwaylandserverbufferintegrationplugin_p.h
        inputdeviceintegration/qwaylandinputdeviceintegration_p.h
        inputdeviceintegration/qwaylandinputdeviceintegrationfactory.cpp inputdeviceintegration/qwaylandinputdeviceintegrationfactory_p.h
        inputdeviceintegration/qwaylandinputdeviceintegrationplugin.cpp inputdeviceintegration/qwaylandinputdeviceintegrationplugin_p.h
        qtwaylandclientglobal.h qtwaylandclientglobal_p.h
        qwaylandabstractdecoration.cpp qwaylandabstractdecoration_p.h
        qwaylandbuffer.cpp qwaylandbuffer_p.h
        qwaylanddecorationfactory.cpp qwaylanddecorationfactory_p.h
        qwaylanddecorationplugin.cpp qwaylanddecorationplugin_p.h
        qwaylanddisplay.cpp qwaylanddisplay_p.h
        qwaylandextendedsurface.cpp qwaylandextendedsurface_p.h
        qwaylandinputcontext.cpp qwaylandinputcontext_p.h
        qwaylandtextinputv1.cpp qwaylandtextinputv1_p.h
        qwaylandtextinputv2.cpp qwaylandtextinputv2_p.h
        qwaylandtextinputinterface.cpp qwaylandtextinputinterface_p.h
        qwaylandinputdevice.cpp qwaylandinputdevice_p.h
        qwaylandinputmethodcontext.cpp qwaylandinputmethodcontext_p.h
        qwaylandintegration.cpp qwaylandintegration_p.h
        qwaylandnativeinterface.cpp qwaylandnativeinterface_p.h
        qwaylandpointergestures.cpp qwaylandpointergestures_p.h
        qwaylandqtkey.cpp qwaylandqtkey_p.h
        qwaylandscreen.cpp qwaylandscreen_p.h
        qwaylandshellsurface.cpp qwaylandshellsurface_p.h
        qwaylandshm.cpp qwaylandshm_p.h
        qwaylandshmbackingstore.cpp qwaylandshmbackingstore_p.h
        qwaylandshmwindow.cpp qwaylandshmwindow_p.h
        qwaylandsubsurface.cpp qwaylandsubsurface_p.h
        qwaylandsurface.cpp qwaylandsurface_p.h
        qwaylandtouch.cpp qwaylandtouch_p.h
        qwaylandwindow.cpp qwaylandwindow_p.h
        qwaylandwindowmanagerintegration.cpp qwaylandwindowmanagerintegration_p.h
        shellintegration/qwaylandclientshellapi_p.h
        shellintegration/qwaylandshellintegration_p.h shellintegration/qwaylandshellintegration.cpp
        shellintegration/qwaylandshellintegrationfactory.cpp shellintegration/qwaylandshellintegrationfactory_p.h
        shellintegration/qwaylandshellintegrationplugin.cpp shellintegration/qwaylandshellintegrationplugin_p.h
    INCLUDE_DIRECTORIES
        ../shared
        global
        hardwareintegration
        inputdeviceintegration
        shellintegration
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandGlobalPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Wayland::Client
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandGlobalPrivate
    PRIVATE_HEADER_FILTERS
        "^qwayland-.*\.h|^wayland-.*-protocol\.h"
)

qt6_generate_wayland_protocol_client_sources(WaylandClient
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/pointer-gestures-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/tablet-unstable-v2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/text-input-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/text-input-unstable-v2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/text-input-unstable-v4-wip.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wp-primary-selection-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/xdg-output-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-key-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-text-input-method-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-windowmanager.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/surface-extension.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/touch-extension.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/hardwareintegration/../../extensions/hardware-integration.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/hardwareintegration/../../extensions/server-buffer-extension.xml
)

#### Keys ignored in scope 1:.:.:client.pro:<TRUE>:
# MODULE = "waylandclient"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"

## Scopes:
#####################################################################

# special case begin
#
# Do not explicitly add XKB::XKB. This is already done by Qt::GuiPrivate.
#
# qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_xkbcommon
#     PUBLIC_LIBRARIES
#         XKB::XKB
# )
# special case end

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_tabletevent
    SOURCES
        qwaylandtabletv2.cpp qwaylandtabletv2_p.h
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_wayland_text_input_v4_wip
    SOURCES
        qwaylandtextinputv4.cpp qwaylandtextinputv4_p.h
    DEFINES
        QT_WAYLAND_TEXT_INPUT_V4_WIP=1
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_clipboard
    SOURCES
        qwaylandclipboard.cpp qwaylandclipboard_p.h
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_vulkan
    SOURCES
        qwaylandvulkaninstance.cpp qwaylandvulkaninstance_p.h
        qwaylandvulkanwindow.cpp qwaylandvulkanwindow_p.h
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_cursor
    SOURCES
        qwaylandcursor.cpp qwaylandcursor_p.h
    PUBLIC_LIBRARIES
        Wayland::Cursor
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_wayland_datadevice
    SOURCES
        qwaylanddatadevice.cpp qwaylanddatadevice_p.h
        qwaylanddatadevicemanager.cpp qwaylanddatadevicemanager_p.h
        qwaylanddataoffer.cpp qwaylanddataoffer_p.h
        qwaylanddatasource.cpp qwaylanddatasource_p.h
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_wayland_client_primary_selection
    SOURCES
        qwaylandprimaryselectionv1.cpp qwaylandprimaryselectionv1_p.h
)

qt_internal_extend_target(WaylandClient CONDITION QT_FEATURE_draganddrop
    SOURCES
        qwaylanddnd.cpp qwaylanddnd_p.h
)

qt_record_extra_qt_main_tools_package_dependency(WaylandClient WaylandScannerTools "${PROJECT_VERSION}")
qt_record_extra_qt_package_dependency(WaylandClient WaylandGlobalPrivate "${PROJECT_VERSION}")
