

#### Inputs



#### Libraries

if((LINUX) OR QT_FIND_ALL_PACKAGES_ALWAYS)
    qt_find_package(Wayland PROVIDED_TARGETS Wayland::Client)
endif()
if((LINUX) OR QT_FIND_ALL_PACKAGES_ALWAYS)
    qt_find_package(Wayland PROVIDED_TARGETS Wayland::Cursor)
endif()
if((LINUX) OR QT_FIND_ALL_PACKAGES_ALWAYS)
    qt_find_package(Wayland PROVIDED_TARGETS Wayland::Egl)
endif()
qt_find_package(XComposite PROVIDED_TARGETS PkgConfig::XComposite)


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
"
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



#### Features

qt_feature("wayland_client" PRIVATE
    LABEL "Qt Wayland Client"
    CONDITION NOT WIN32 AND Wayland_FOUND AND Wayland_FOUND AND WaylandScanner_FOUND
)
qt_feature("wayland_datadevice" PRIVATE
    CONDITION QT_FEATURE_draganddrop OR QT_FEATURE_clipboard
)
qt_feature("wayland_client_primary_selection" PRIVATE
    LABEL "primary-selection clipboard"
    CONDITION QT_FEATURE_clipboard
)
qt_feature("wayland_client_fullscreen_shell_v1" PRIVATE
    LABEL "fullscreen-shell-v1"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_client_ivi_shell" PRIVATE
    LABEL "ivi-shell"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_client_wl_shell" PRIVATE
    LABEL "wl-shell (deprecated)"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_client_xdg_shell" PRIVATE
    LABEL "xdg-shell"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_client_xdg_shell_v5" PRIVATE
    LABEL "xdg-shell unstable v5 (deprecated)"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_client_xdg_shell_v6" PRIVATE
    LABEL "xdg-shell unstable v6"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland_egl" PRIVATE
    LABEL "EGL"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND Wayland_FOUND
)
qt_feature("wayland_brcm" PRIVATE
    LABEL "Raspberry Pi"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_eglfs_brcm
)
qt_feature("xcomposite_egl" PRIVATE
    LABEL "XComposite EGL"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND QT_FEATURE_xlib AND XComposite_FOUND AND QT_FEATURE_egl_x11
)
qt_feature("xcomposite_glx" PRIVATE
    LABEL "XComposite GLX"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND NOT QT_FEATURE_opengles2 AND QT_FEATURE_xlib AND XComposite_FOUND
)
qt_feature("wayland_drm_egl_server_buffer" PRIVATE
    LABEL "DRM EGL"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_drm_egl_server
)
qt_feature("wayland_libhybris_egl_server_buffer" PRIVATE
    LABEL "libhybris EGL"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_libhybris_egl_server
)
qt_feature("wayland_dmabuf_server_buffer" PRIVATE
    LABEL "Linux dma-buf server buffer integration"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_dmabuf_server_buffer
)
qt_feature("wayland_vulkan_server_buffer" PRIVATE
    LABEL "Vulkan-based server buffer integration"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_vulkan_server_buffer
)
qt_feature("wayland_shm_emulation_server_buffer" PRIVATE
    LABEL "Shm emulation server buffer integration"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl
)
