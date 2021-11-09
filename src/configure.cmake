# configure.cmake for the QtWaylandGlobalPrivate module

#### Inputs
set(INPUT_wayland_text_input_v4_wip OFF CACHE BOOL "")



#### Libraries

if(LINUX OR QT_FIND_ALL_PACKAGES_ALWAYS)
    # waylandclient libraries
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Client
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-client)
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Cursor
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-cursor)
    qt_add_qmake_lib_dependency(wayland-cursor wayland-client)
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Egl
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-egl)

    # waylandcompositor libraries
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Server
        MODULE_NAME waylandcompositor
        QMAKE_LIB wayland-server)
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Egl
        MODULE_NAME waylandcompositor
        QMAKE_LIB wayland-egl)

    # X11 is not a public dependency of QtGui anymore, so we need to find it manually in a shared build.
    # In a static build the dependency is still propagated, so check for the target existence to prevent
    # the 'Attempt to promote imported target "X11::X11" to global scope' issue.
    if(NOT TARGET X11::X11)
        qt_find_package(X11 PROVIDED_TARGETS X11::X11 MODULE_NAME gui QMAKE_LIB xlib)
    endif()
    # Same for XKB.
    if(NOT TARGET XKB::XKB)
        qt_find_package(XKB 0.5.0 PROVIDED_TARGETS XKB::XKB MODULE_NAME gui QMAKE_LIB xkbcommon MARK_OPTIONAL)
    endif()
endif()


#### Tests

# drm-egl-server
qt_config_compile_test(drm_egl_server
    LABEL "DRM EGL Server"
    LIBRARIES
    EGL::EGL
    CODE
    "
#include <EGL/egl.h>
#include <EGL/eglext.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
#ifdef EGL_MESA_drm_image
return 0;
#else
#error Requires EGL_MESA_drm_image to be defined
return 1;
#endif
    /* END TEST: */
    return 0;
}
")

# libhybris-egl-server
qt_config_compile_test(libhybris_egl_server
    LABEL "libhybris EGL Server"
    LIBRARIES
    EGL::EGL
    CODE
    "
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <hybris/eglplatformcommon/hybris_nativebufferext.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
#ifdef EGL_HYBRIS_native_buffer
return 0;
#else
#error Requires EGL_HYBRIS_native_buffer to be defined
return 1;
#endif
    /* END TEST: */
    return 0;
}
")

# dmabuf-server-buffer
qt_config_compile_test(dmabuf_server_buffer
    LABEL "Linux dma-buf Buffer Sharing"
    LIBRARIES
    EGL::EGL
    Libdrm::Libdrm
    CODE
    "
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <drm_fourcc.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
#ifdef EGL_LINUX_DMA_BUF_EXT
return 0;
#else
#error Requires EGL_LINUX_DMA_BUF_EXT
return 1;
#endif
    /* END TEST: */
    return 0;
}
")

# vulkan-server-buffer
qt_config_compile_test(vulkan_server_buffer
    LABEL "Vulkan Buffer Sharing"
    CODE
    "#define VK_USE_PLATFORM_WAYLAND_KHR 1
#include <vulkan/vulkan.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
VkExportMemoryAllocateInfoKHR exportAllocInfo = {};
exportAllocInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
exportAllocInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
return 0;
    /* END TEST: */
    return 0;
}
")

# egl_1_5-wayland
qt_config_compile_test(egl_1_5_wayland
    LABEL "EGL 1.5 with Wayland Platform"
    LIBRARIES
    EGL::EGL
    CODE
    "
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-client.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, (struct wl_display *)(nullptr), nullptr);
    /* END TEST: */
    return 0;
}
")


#### Features

qt_feature("wayland-client" PRIVATE
    LABEL "Qt Wayland Client"
    CONDITION NOT WIN32 AND Wayland_FOUND AND WaylandScanner_FOUND
)
qt_feature("wayland-server" PRIVATE
    LABEL "Qt Wayland Compositor"
    CONDITION NOT WIN32 AND Wayland_FOUND AND WaylandScanner_FOUND
)
qt_feature("wayland-egl" PRIVATE
    LABEL "EGL"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server)
              AND QT_FEATURE_opengl AND QT_FEATURE_egl
              AND (NOT QNX OR QT_FEATURE_egl_extension_platform_wayland)
)
qt_feature("wayland-brcm" PRIVATE
    LABEL "Raspberry Pi"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_eglfs_brcm
)
qt_feature("wayland-drm-egl-server-buffer" PRIVATE
    LABEL "DRM EGL"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_opengl
              AND QT_FEATURE_egl AND TEST_drm_egl_server
              AND (NOT QNX OR QT_FEATURE_egl_extension_platform_wayland)
)
qt_feature("wayland-libhybris-egl-server-buffer" PRIVATE
    LABEL "libhybris EGL"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_opengl
              AND QT_FEATURE_egl AND TEST_libhybris_egl_server
)
qt_feature("wayland-dmabuf-server-buffer" PRIVATE
    LABEL "Linux dma-buf server buffer integration"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_opengl
              AND QT_FEATURE_egl AND TEST_dmabuf_server_buffer
)
qt_feature("wayland-shm-emulation-server-buffer" PRIVATE
    LABEL "Shm emulation server buffer integration"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_opengl
)
qt_feature("wayland-vulkan-server-buffer" PRIVATE
    LABEL "Vulkan-based server buffer integration"
    CONDITION (QT_FEATURE_wayland_client OR QT_FEATURE_wayland_server) AND QT_FEATURE_vulkan
              AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_vulkan_server_buffer
)
qt_feature("wayland-datadevice" PRIVATE
    CONDITION QT_FEATURE_draganddrop OR QT_FEATURE_clipboard
)
qt_feature("wayland-text-input-v4-wip" PRIVATE
    LABEL "Qt Wayland TextInput Protocol V4(WIP)"
    PURPOSE "Enables wayland_text_input_unstable_v4(wip)"
)

qt_configure_add_summary_entry(ARGS "wayland-text-input-v4-wip")
qt_configure_add_summary_entry(ARGS "wayland-client")
qt_configure_add_summary_entry(ARGS "wayland-server")
qt_configure_add_summary_section(NAME "Qt Wayland Drivers")
qt_configure_add_summary_entry(ARGS "wayland-egl")
qt_configure_add_summary_entry(ARGS "wayland-brcm")
qt_configure_add_summary_entry(ARGS "wayland-drm-egl-server-buffer")
qt_configure_add_summary_entry(ARGS "wayland-libhybris-egl-server-buffer")
qt_configure_add_summary_entry(ARGS "wayland-dmabuf-server-buffer")
qt_configure_add_summary_entry(ARGS "wayland-shm-emulation-server-buffer")
qt_configure_add_summary_entry(ARGS "wayland-vulkan-server-buffer")
qt_configure_end_summary_section() # end of "Qt Wayland Drivers" section
