#### Inputs



#### Libraries


#### Tests


#### Features

qt_feature("wayland-client-primary-selection" PRIVATE
    LABEL "primary-selection clipboard"
    CONDITION QT_FEATURE_clipboard
)
qt_feature("wayland-client-fullscreen-shell-v1" PRIVATE
    LABEL "fullscreen-shell-v1"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland-client-ivi-shell" PRIVATE
    LABEL "ivi-shell"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland-client-wl-shell" PRIVATE
    LABEL "wl-shell (deprecated)"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland-client-xdg-shell" PRIVATE
    LABEL "xdg-shell"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("egl-extension-platform-wayland" PRIVATE
    LABEL "EGL wayland platform extension"
    CONDITION QT_FEATURE_wayland_client AND QT_FEATURE_opengl AND QT_FEATURE_egl AND TEST_egl_1_5_wayland
)
qt_configure_add_summary_section(NAME "Qt Wayland Client Shell Integrations")
qt_configure_add_summary_entry(ARGS "wayland-client-xdg-shell")
qt_configure_add_summary_entry(ARGS "wayland-client-xdg-shell-v5")
qt_configure_add_summary_entry(ARGS "wayland-client-xdg-shell-v6")
qt_configure_add_summary_entry(ARGS "wayland-client-ivi-shell")
qt_configure_add_summary_entry(ARGS "wayland-client-wl-shell")
qt_configure_end_summary_section() # end of "Qt Wayland Client Shell Integrations" section
