# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Inputs



#### Libraries

if((LINUX) OR QT_FIND_ALL_PACKAGES_ALWAYS)
    qt_find_package(Waylandkms PROVIDED_TARGETS PkgConfig::Waylandkms MODULE_NAME waylandcompositor QMAKE_LIB wayland-kms)
endif()
# special case begin

# Even if libdrm is already found by qtbase we still need to list it as dependency for some of our
# plugins
if(TARGET Libdrm::Libdrm)
    qt_internal_disable_find_package_global_promotion(Libdrm::Libdrm)
endif()
qt_find_package(Libdrm PROVIDED_TARGETS Libdrm::Libdrm MODULE_NAME gui QMAKE_LIB drm MARK_OPTIONAL)


#### Tests

# dmabuf-client-buffer
qt_config_compile_test(dmabuf_client_buffer
    LABEL "Linux Client dma-buf Buffer Sharing"
    LIBRARIES
        EGL::EGL
        Libdrm::Libdrm
    CODE
"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <drm_mode.h>
#include <drm_fourcc.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
// test if DMA BUF is supported
#ifndef EGL_LINUX_DMA_BUF_EXT
#error DMA BUF Extension not available
#endif
// test if DMA BUF import modifier extension is supported
#ifndef EGL_EXT_image_dma_buf_import_modifiers
#error DMA BUF Import modifier extension not available
#endif
return 0;
    /* END TEST: */
    return 0;
}
")


#### Features

qt_feature("wayland-dmabuf-client-buffer" PRIVATE
    LABEL "Linux dma-buf client buffer integration"
    CONDITION QT_FEATURE_wayland_server AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_dmabuf_client_buffer
)
qt_feature("wayland-layer-integration-vsp2" PRIVATE
    LABEL "VSP2 hardware layer integration"
    CONDITION QT_FEATURE_wayland_server AND QT_FEATURE_eglfs_vsp2 AND Waylandkms_FOUND
)
qt_feature("wayland-compositor-quick" PUBLIC
    LABEL "QtQuick integration for wayland compositor"
    PURPOSE "Allows QtWayland compositor types to be used with QtQuick"
    CONDITION QT_FEATURE_wayland_server AND TARGET Qt::Quick
)
qt_configure_add_summary_section(NAME "Qt Wayland Compositor Layer Plugins")
qt_configure_add_summary_entry(ARGS "wayland-layer-integration-vsp2")
qt_configure_end_summary_section() # end of "Qt Wayland Compositor Layer Plugins" section
