include(FindPkgConfig)

pkg_check_modules(Waylandkms wayland-kms IMPORTED_TARGET)

if (NOT TARGET PkgConfig::Waylandkms)
    set(Waylandkms_FOUND 0)
endif()
