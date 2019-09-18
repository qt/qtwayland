include(FindPkgConfig)

pkg_check_modules(XComposite xcomposite IMPORTED_TARGET)

if (NOT TARGET PkgConfig::XComposite)
    set(XComposite_FOUND 0)
endif()
